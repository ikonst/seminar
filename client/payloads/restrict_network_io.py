try:
    from http.client import HTTPConnection
    conn = HTTPConnection('127.0.0.1', port=80)
    conn.connect()
    raise ValueError('Failed to restrict network I/O')
except PermissionError:
    print('Successfully protected against network connection')
