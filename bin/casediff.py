#!/usr/bin/python3

# A program with an interface similar to diff that compares two files case-by-case.
# A case is a portion of the file made up of one or more lines, where the first line
# is a *header* that matches the *case pattern*.
#
# Files to be compared default to program.txt and correct.txt
#
# The files are considered equal if all cases are considered equal. Two cases are
# considered equal if either their header lines compare equal (--equal-header) or
# they have the same number of lines and each of their corresponding lines is equal
# (--equal-body, the default).
# Lines can be compared byte-by-byte (like diff), token-by-token or numerically.

import sys
import re
import argparse
import os

PATTERN = r'^Case #(\d+):'
EPSILON = 0
METHOD = 'numeric'  # exact, token, numeric
FORMATTING = 'normal'  # normal, sidebyside, caseno, brief
ONECASE = 0
WRAP_LENGTH = 120

RED, GREEN, YELLOW, COLOR_RESET = '\033[1;31m', '\033[1;32m', '\033[1;34m', '\033[0m'


class CasediffErrors:
    def io_error(file, err):
        print("casediff: {}: {}".format(file, err), file=sys.stderr)
        sys.exit(2)

    def first_line_error(file):
        print("casediff: {}: 1st line doesn't match pattern".format(file),
              file=sys.stderr)
        sys.exit(2)

    def wrong_caseno_error(file, lineno, caseno, expected_caseno):
        print("casediff: {}:{}: expected case #{} but got #{}".format(
            file, lineno, expected_caseno, caseno),
            file=sys.stderr)
        sys.exit(2)

    def caseno_count_warn(a_file, a_casenos, b_file, b_casenos):
        print("casediff: different number of cases: {} ({}) vs {} ({})".format(
            a_casenos, a_file, b_casenos, b_file),
            file=sys.stderr)

    def no_cases_warn(file_a, file_b):
        print("casediff: no cases found in either {} or {}".format(
            file_a, file_b),
            file=sys.stderr)


class WordSplitter:
    def __init__(self, line):
        self.words = re.split(r'(\S+)', line)
        self.i = -1
        self.n = len(self.words) // 2
        assert len(self.words) % 2 == 1

    def trailing_space(self):
        return self.words[-1]

    def __iter__(self):
        return self

    def __next__(self):
        self.i += 1
        if self.i >= self.n:
            self.i = -1
            raise StopIteration
        else:
            return (self.words[2 * self.i], self.words[2 * self.i + 1])


def casediff_parse_args():
    parser = argparse.ArgumentParser(
        description='Compare two output casefiles, side by side\n'
        'Default pattern appropriate for kickstart/codejam/facebook',
        epilog='Exit status is 0 if equal, 1 if different, 2 if trouble.\n'
        'By default each case should match numerically.\n'
        'Diffs are coloured by default.',
        fromfile_prefix_chars='@',
        allow_abbrev=False,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )

    DEFAULT_PATTERN = os.getenv('PATTERN', PATTERN)
    DEFAULT_EPSILON = os.getenv('EPSILON', EPSILON)
    DEFAULT_ONECASE = os.getenv('ONECASE', ONECASE)
    DEFAULT_METHOD = os.getenv('METHOD', METHOD)
    DEFAULT_FORMATTING = os.getenv('FORMATTING', FORMATTING)

    files = parser.add_argument_group('Input files').add_argument
    files('lhs_file', nargs='?', default='output.txt', help='LHS casefile')
    files('rhs_file', nargs='?', default='answer.txt', help='Rhs casefile')

    comp = parser.add_argument_group('Comparison').add_argument
    comp('-P',
         '--pattern',
         nargs=1,
         action='store',
         default=DEFAULT_PATTERN,
         metavar='PATTERN',
         help='Case header regex pattern (capture group for case number)')
    comp('-1',
         '--one',
         action='store_true',
         default=DEFAULT_ONECASE,
         help='Treat whole thing as one case and ignore pattern')
    comp('--equal-header',
         action='store_true',
         default=False,
         help='Only headers must match')
    comp('--equal-body',
         action='store_false',
         dest='equal_header',
         help='Everything must match')
    comp('--exact',
         action='store_const',
         const='exact',
         dest='method',
         default=DEFAULT_METHOD,
         help='Compare lines byte by byte')
    comp('--token',
         action='store_const',
         const='token',
         dest='method',
         help='Compare lines token by token')
    comp('--numeric',
         action='store_const',
         const='numeric',
         dest='method',
         help='Compare lines numerically')
    comp('-e',
         '--epsilon',
         type=float,
         default=DEFAULT_EPSILON,
         metavar='EPS',
         help='Precision error allowed (abs or rel)')

    fmt = parser.add_argument_group('Formatting').add_argument
    fmt('--normal',
        action='store_const',
        const='normal',
        dest='formatting',
        default=DEFAULT_FORMATTING,
        help='Output a normal diff (not side by side)')
    fmt('-y',
        '--side-by-side',
        action='store_const',
        const='sydebyside',
        dest='formatting',
        help='Output in two columns')
    fmt('--caseno',
        action='store_const',
        const='caseno',
        dest='formatting',
        help='Output a newline-separated list of mismatching (bad) casenos')
    fmt('-q',
        '--brief',
        action='store_const',
        const='brief',
        dest='formatting',
        help='Do not output diffs, only report status')
    fmt('-s',
        '--summary',
        action='store_true',
        default=False,
        help='Print a summary count of good cases at the end')
    fmt('--suppress',
        '--suppress-common-lines',
        action='store_true',
        dest='suppress',
        default=False,
        help="Do not output common lines")
    fmt('--color', action='store_true', default=True, help='Color diffs')
    fmt('--no-color', action='store_false', dest='color')

    args = parser.parse_args()
    args.exact = args.method == 'exact'
    args.tokenize = args.method == 'token'
    args.numeric = args.method == 'numeric'
    args.normal = args.formatting == 'normal'
    args.sidebyside = args.formatting == 'sidebyside'
    args.caseno = args.formatting == 'caseno'
    args.brief = args.formatting == 'brief'
    return args


def read_as_one_case(file):
    try:
        with open(file, "r") as f:
            body = [line for line in f]
            return [(body[0], body, 1)] if len(body) > 0 else []
    except IOError as err:
        CasediffErrors.io_error(file, err)


def read_cases(file, pattern):
    caseno, headers, bodies, linenos = 0, [], [], []
    if not isinstance(pattern, re.Pattern):
        pattern = re.compile(pattern)
    try:
        with open(file, "r") as f:
            for lineno, line in enumerate(f):
                line = line[:-1] if len(
                    line) > 0 and line[-1] == '\n' else line
                match = pattern.match(line)
                if match:
                    caseno += 1
                    actual_caseno = int(match.group(1))
                    if actual_caseno != caseno:
                        CasediffErrors.wrong_caseno_error(
                            file, lineno, actual_caseno, caseno)
                    prefix_len = len(match[0])
                    bodies.append([line])
                    headers.append(line[prefix_len:])
                    linenos.append(lineno + 1)
                elif caseno == 0:
                    CasediffErrors.first_line_error(file)
                else:
                    bodies[-1].append(line)
    except IOError as err:
        CasediffErrors.io_error(file, err)

    return [(headers[i], bodies[i], linenos[i]) for i in range(caseno)]


def is_num_str(s):
    try:
        x = float(s)
        return True
    except:
        return False


def equal_numeric(a, b, epsilon=0):
    return a == b or is_num_str(a) and is_num_str(b) and \
        abs(float(a) - float(b)) <= epsilon or \
        (abs(float(a) - float(b))) / (abs(float(a)) + abs(float(b))) <= epsilon


def equal_lines_word(aline, bline):
    a_words, b_words = WordSplitter(aline), WordSplitter(bline)
    return a_words.n == b_words.n and \
        all(a[1] == b[1] for a, b in zip(a_words, b_words))


def equal_lines_numeric(aline, bline, epsilon=0):
    a_words, b_words = WordSplitter(aline), WordSplitter(bline)
    return a_words.n == b_words.n and \
        all(equal_numeric(a[1], b[1], epsilon)
            for a, b in zip(a_words, b_words))


def equal_lines(aline, bline, opts):
    if opts.numeric:
        return equal_lines_numeric(aline, bline, opts.epsilon)
    elif opts.tokenize:
        return equal_lines_word(aline, bline)
    else:
        return aline == bline


def format_difflines_text(aline, bline):
    ok, a_diff, b_diff = True, "", ""
    for a_char, b_char in zip(aline, bline):
        if a_char == b_char and not ok:
            a_diff += COLOR_RESET
            b_diff += COLOR_RESET
        elif a_char != b_char and ok:
            a_diff += YELLOW
            b_diff += YELLOW
        a_diff += a_char
        b_diff += b_char
        ok = a_char == b_char
    if not ok:
        a_diff += COLOR_RESET
        b_diff += COLOR_RESET
    if len(aline) > len(bline):
        a_diff += RED + aline[len(bline):] + COLOR_RESET
    if len(aline) < len(bline):
        b_diff += GREEN + bline[len(aline):] + COLOR_RESET
    return a_diff, b_diff


def format_difflines_word(aline, bline):
    a_diff, b_diff = "", ""
    a_words, b_words = WordSplitter(aline), WordSplitter(bline)
    for (a_space, a_word), (b_space, b_word) in zip(a_words, b_words):
        if a_word == b_word:
            a_diff += a_space + a_word
            b_diff += b_space + b_word
        else:
            a_wordiff, b_wordiff = format_difflines_text(a_word, b_word)
            a_diff += a_space + a_wordiff
            b_diff += b_space + b_wordiff
    if a_words.n > b_words.n:
        for (a_space, a_word) in a_words:
            a_diff += a_space + RED + a_word + COLOR_RESET
    elif a_words.n < b_words.n:
        for (b_space, b_word) in b_words:
            b_diff += b_space + GREEN + b_word + COLOR_RESET
    a_diff += a_words.trailing_space()
    b_diff += b_words.trailing_space()
    return a_diff, b_diff


def format_difflines_numeric(aline, bline, epsilon):
    a_diff, b_diff = "", ""
    a_words, b_words = WordSplitter(aline), WordSplitter(bline)
    for (a_space, a_word), (b_space, b_word) in zip(a_words, b_words):
        if equal_numeric(a_word, b_word, epsilon):
            a_diff += a_space + a_word
            b_diff += b_space + b_word
        elif is_num_str(a_word) or is_num_str(b_word):
            a_diff += a_space + YELLOW + a_word + COLOR_RESET
            b_diff += b_space + YELLOW + b_word + COLOR_RESET
        else:
            a_wordiff, b_wordiff = format_difflines_text(a_word, b_word)
            a_diff += a_space + a_wordiff
            b_diff += b_space + b_wordiff
    if a_words.n > b_words.n:
        for (a_space, a_word) in a_words:
            a_diff += a_space + RED + a_word + COLOR_RESET
    elif a_words.n < b_words.n:
        for (b_space, b_word) in b_words:
            b_diff += b_space + GREEN + b_word + COLOR_RESET
    a_diff += a_words.trailing_space()
    b_diff += b_words.trailing_space()
    return a_diff, b_diff


def format_diffline(aline, bline, opts):
    if opts.color:
        if opts.numeric:
            return format_difflines_numeric(aline, bline, opts.epsilon)
        elif opts.tokenize:
            return format_difflines_word(aline, bline)
        else:
            return format_difflines_text(aline, bline)
    else:
        return aline, bline


def format_extraline(line, color, opts):
    if opts.color:
        return color + line + COLOR_RESET
    else:
        return line


class Diffline:
    def __init__(self, caseno, i, linenos, lines, diffs):
        self.caseno = caseno
        self.index = i
        self.lineno = [(n + i) if n >= 0 else 0 for n in linenos]
        self.line = lines
        self.diff = diffs
        self.width = (len(lines[0]), len(lines[1]))


def equal_cases(a, b, opts):
    a_header, alines, _ = a
    b_header, blines, _ = b

    if opts.equal_header:
        return equal_lines(a_header, b_header, opts)
    else:
        return len(alines) == len(blines) and all(
            equal_lines(aline, bline, opts)
            for aline, bline in zip(alines, blines))


def make_case_difflines(caseno, a, b, opts):
    _, alines, alineno = a
    _, blines, blineno = b
    n, m = min(len(alines), len(blines)), max(len(alines), len(blines))

    eqs = [
        equal_lines(aline, bline, opts)
        for aline, bline in zip(alines, blines)
    ]

    diffs = [
        Diffline(caseno, i, (alineno, blineno), (aline, bline),
                 format_diffline(aline, bline, opts))
        for i, (aline, bline) in enumerate(zip(alines, blines))
        if not opts.suppress or not eqs[i]
    ]

    if len(alines) > len(blines):
        diffs += [
            Diffline(caseno, i + len(blines), (alineno, -1), (aline, ""),
                     (format_extraline(aline, RED, opts), ""))
            for i, aline in enumerate(alines[len(blines):])
            if not opts.suppress or not eqs[i]
        ]
    if len(alines) < len(blines):
        diffs += [
            Diffline(caseno, i + len(alines), (-1, blineno), ("", bline),
                     ("", format_extraline(bline, GREEN, opts)))
            for i, bline in enumerate(blines[len(alines):])
            if not opts.suppress or not eqs[i]
        ]

    return diffs


def print_difflines(diffs, opts):
    if len(diffs) == 0:
        return

    lineno_align = [
        max(len(str(diff.lineno[s])) for diff in diffs) for s in range(2)
    ]
    text_align = [max(diff.width[0] for diff in diffs), 0]
    pad = " " * (lineno_align[0] + 2 + text_align[0])
    total_len = sum(lineno_align) + sum(text_align) + 4
    spacing = " " * 6 if total_len <= WRAP_LENGTH - 6 else "\n\t"

    def build(diff, s):
        spaces = text_align[s] - diff.width[s]
        return "{:>{}}: {}{}".format(diff.lineno[s], lineno_align[s],
                                     diff.diff[s], " " * spaces)

    if opts.sidebyside and total_len <= WRAP_LENGTH - 6:
        for diff in diffs:
            if diff.width[0] and diff.width[1]:
                print("{}{}{}".format(build(diff, 0), spacing, build(diff, 1)))
            elif diff.width[1]:
                print("{}{}{}".format(pad, spacing, build(diff, 1)))
            else:
                print("{}".format(build(diff, 0)))
    else:
        for diff in diffs:
            if diff.width[0] and diff.width[1]:
                print("< {}\n> {}".format(build(diff, 0), build(diff, 1)))
            elif diff.width[1]:
                print("> {}".format(build(diff, 1)))
            else:
                print("< {}".format(build(diff, 0)))


def run_casediff():
    opts = casediff_parse_args()
    a_file, b_file = opts.lhs_file, opts.rhs_file

    if opts.one:
        a_cases, b_cases = read_as_one_case(a_file), read_as_one_case(b_file)
    else:
        pat = opts.pattern
        a_cases, b_cases = read_cases(a_file, pat), read_cases(b_file, pat)

    a_casenos, b_casenos = len(a_cases), len(b_cases)

    if a_casenos != b_casenos:
        CasediffErrors.caseno_count_warn(a_file, a_casenos, b_file, b_casenos)
    elif b_casenos == 0:
        CasediffErrors.no_cases_warn(a_file, b_file)

    casenos = min(a_casenos, b_casenos)
    equal = [equal_cases(a, b, opts) for a, b in zip(a_cases, b_cases)]
    good = sum(equal)

    if opts.caseno:
        for i in range(casenos):
            if not equal[i]:
                print(i + 1)

    if opts.sidebyside or opts.normal:
        diffs = []
        for i, (a, b) in enumerate(zip(a_cases, b_cases)):
            if not equal[i]:
                diffs += make_case_difflines(i + 1, a, b, opts)
        print_difflines(diffs, opts)

    if opts.summary and casenos > 0:
        percentage = 100 * good / casenos
        print("Correct: {}/{} ({:.1f}%)".format(good, casenos, percentage))

    return good == casenos


if __name__ == "__main__":
    equal = run_casediff()
    sys.exit(0 if equal else 1)
