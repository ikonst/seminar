#!/usr/bin/env python -u
import hashlib
import logging
import os
import sys
import traceback
from contextlib import contextmanager
from resource import getrlimit, setrlimit, RLIMIT_NOFILE
from typing import Tuple, List, Optional, Any

import gevent
from socket import socket
from gevent import subprocess
from gevent.fileobject import FileObjectPosix
from gevent.server import StreamServer
from gevent.ssl import wrap_socket

import settings


log = logging.getLogger(__name__)
connections_count = 0


def _addr() -> Tuple[str, str]:
    greenlet = gevent.getcurrent()
    return getattr(greenlet, 'address', ('unknown', 'unknown'))


def _print(*args, **kwargs):
    host, port = _addr()
    print(f'{host}:{port}', *args, **kwargs)


def track_connection(sock: socket, address: Tuple[str, int]):
    global connections_count
    connections_count += 1
    gevent.getcurrent().address = address
    _print(f"New connection; currently have {connections_count} connections")
    try:
        handle_connection(sock)
    finally:
        connections_count -= 1


def challenge_client(sock: socket) -> bool:
    """
    Implements a SHA-256-based challenge-response over a socket.
    """
    nonce = os.urandom(32)
    sock.send(nonce)
    response = sock.recv(32)
    expected_response = hashlib.sha256(nonce + settings.PASSPHRASE).digest()
    return response == expected_response


def pipe_socket(sock: socket, buffer_size: int=1) -> Tuple[int, int]:
    """
    Creates a pipe that's reading and writing to a socket.
    This is useful e.g. for piping SSL connections' (plaintext) contents to subprocesses.
    """
    address = _addr()
    read_fd, stdout_fd = os.pipe()
    stdin_fd, write_fd = os.pipe()

    def _reader():
        f = FileObjectPosix(write_fd, 'wb', bufsize=0, close=True)
        while True:
            data = sock.recv(buffer_size)
            if len(data) == 0:
                return
            f.write(data)

    def _writer():
        stdout = FileObjectPosix(read_fd, 'rb', bufsize=0, close=True)
        while True:
            data = stdout.read(buffer_size)
            if len(data) == 0:
                return
            sock.write(data)

    reader_greenlet = gevent.spawn(_reader)
    reader_greenlet.address = address
    reader_greenlet.link_exception(lambda greenlet: sys.exit(1))

    writer_greenlet = gevent.spawn(_writer)
    writer_greenlet.address = address
    writer_greenlet.link_exception(lambda greenlet: sys.exit(1))

    return stdin_fd, stdout_fd


def read_code(sock: socket) -> str:
    f = sock.makefile(mode='rwb', buffering=0)
    code = ''
    while True:
        line = f.readline().decode('utf-8')
        if line in ['END CODE\n']:
            break
        if line in ['', 'END CODE\n']:
            break
        code += line
    return code


def handle_connection(sock: socket):
    if not challenge_client(sock):
        _print('Challenge failed')
        gevent.sleep(2000)
        sock.close()
        return

    _print('Challenge OK!')
    sock = wrap_socket(sock, server_side=True, certfile='server.pem')

    try:
        code = read_code(sock)
        stdin_fd, stdout_fd = pipe_socket(sock)

        _print("Start executing received code in sandbox")
        with execute_code_in_sandbox(code, stdin=stdin_fd, stdout=stdout_fd, stderr=None, max_cpu_time=2) as p:
            retval = p.wait()
            if retval == -24:
                print("resource_limit_exceeded\n")
                f.write(b'ERROR resource_limit_exceeded\n')
            elif retval != 0:
                print(f"Code execution returned ERROR {retval}\n")
            else:
                print(f"Code execution returned OK = {retval}\n")
    except Exception as ex:
        _print(traceback.format_exc())
        sys.stdout.write(f"Exception: {type(ex).__name__}\n")
        f.write(b'ERROR exception\n')


@contextmanager
def execute_code_in_sandbox(code_str: str, stdin: Any, stdout: Any, stderr: Any, *,
                            max_cpu_time: int=None) -> subprocess.Popen:
    if max_cpu_time:
        code_str = (
            "import resource\n"
            f"resource.setrlimit(resource.RLIMIT_CPU, [{max_cpu_time}, {max_cpu_time}])\n"
        ) + code_str
    read_fd, write_fd = os.pipe()
    try:
        args = [
            # unbuffered output
            '-u',
            # provide the program code over the pipe
            f'/dev/fd/{read_fd}',
        ]
        with popen_in_sandbox(profile_file="profile.sb",
                              command=sys.executable,
                              args=args,
                              stdin=stdin,
                              stdout=stdout,
                              stderr=stderr,
                              pass_fds=(read_fd,)) as proc:
            os.write(write_fd, code_str.encode('utf-8'))
            os.close(write_fd)
            yield proc
    finally:
        os.close(read_fd)


def popen_in_sandbox(
        *,
        profile_file: Optional[str]=None,
        profile_name: Optional[str]=None,
        command: str, args: List[str],
        stdin: Any,
        stdout: Any,
        stderr: Any,
        pass_fds: Tuple[int, ...] = None,
) -> subprocess.Popen:
    _args = ["/usr/bin/sandbox-exec"]
    _args += ['-D', f'VIRTUAL_ENV={os.getenv("VIRTUAL_ENV")}']
    if profile_file:
        _args += ["-f", profile_file]
    if profile_name:
        _args += ["-n", profile_name]
    _args.append(command)
    _args += args
    return subprocess.Popen(args=_args, stdin=stdin, stdout=stdout, stderr=stderr, pass_fds=pass_fds, bufsize=0)


def fix_max_open_files():
    nofile_limit = getrlimit(RLIMIT_NOFILE)
    print(f"Increasing open file/socket limit from {nofile_limit[0]} to {settings.MAX_OPEN_FILES}")
    nofile_limit = (settings.MAX_OPEN_FILES, nofile_limit[1])
    setrlimit(RLIMIT_NOFILE, nofile_limit)


def main():
    print(f"Listening on port {settings.PORT}")
    server = StreamServer(('127.0.0.1', settings.PORT), track_connection)
    server.start()
    server.serve_forever()


if __name__ == '__main__':
    main()
