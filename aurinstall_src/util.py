import os
from aurinstall_src.global_vars import *

def pretty_print(text: str, indent=4, first_indent=4):
    try:
        termwidth = os.get_terminal_size().columns - indent
        words = [i for i in text.split(' ')]

        print(' ' * first_indent, end='')
        i = 0
        for index, word in enumerate(words):
            print(word, end=' ')
            i += len(word) + 1

            try:
                if i + (len(words[index + 1]) + 1) >= termwidth:
                    print('')
                    print(' ' * indent, end='')
                    i = 0

            except IndexError:
                pass

        print('')
    except:
        pass

def all_list_in_str(str_, lst):
    for i in lst:
        if i not in str_:
            return False

    return True

def print_err(errstr: str, prefix='ERROR: '):
    if (normal_term or opts['color'] == SMART) or opts['color'] == True:
        print(f'{RED}{prefix}{ENDC}{errstr}')
    else:
        print(f'{prefix}{errstr}')

def log(err: str, lvl: int):
    if opts['verbosity'] == -1:
        return
    if lvl > 3:
        lvl = 3

    application_log[f'{err}\n'] = lvl

    if lvl <= opts['verbosity']:
        print(err)

def onstart_log():
    log(f'home folder: {home_folder}', 1)
    log(f'cache path: {cache_path}', 1)

    if opts['args_parsed'] == True:
        for opt in opts:
            log(f'option {opt} set to {opts[opt]}', 1)
    else:
        log(f'WARNING: COMMANDS LINE ARGUMENTS HAVE NOT YET BEEN PARSED.', 0)
    