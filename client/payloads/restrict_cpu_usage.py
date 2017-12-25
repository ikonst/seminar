for number in range(1, 100_000_000):
    _ = 1_000_000 / number

raise RuntimeError('Failed to limit CPU usage')
