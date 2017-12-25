try:
    with open("/tmp/foo.txt") as f:
        print(f.read())
        raise RuntimeError('Failed to protect against file I/O')
except PermissionError:
    print('Successfully protected against opening a file')
except IOError:
    raise RuntimeError('Failed to protect against file I/O')
