utf8stdout = open(1, 'w', encoding='utf-8', closefd=False)


def print_to_cpp(string: str):
    print(string, file=utf8stdout, flush=True)