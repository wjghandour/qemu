#!/usr/bin/env python3
import os, sys
import argparse

try:
    import yaml
except ModuleNotFoundError:
    sys.exit("ERROR: module yaml not found. Please install for instance with: 'pip install PyYAML'")

def main():
    parser = argparse.ArgumentParser(description="Compare two test results with some filtering options")
    parser.add_argument("--no-trap", action='store_true', help="Filter out tests that have trapped")
    parser.add_argument("--print-test", action='store_true', help="Print test name that differ")
    parser.add_argument("--print-diff", action='store_true', help="Print diff, implies --print-test")
    parser.add_argument("IN", help="Test results yaml file")
    parser.add_argument("REF", help="Reference results yaml file")
    args = parser.parse_args()

    with open(args.IN) as f:
        in_obj = yaml.safe_load(f)
    with open(args.REF) as f:
        ref_obj = yaml.safe_load(f)
    in_tests, ref_tests = in_obj['TESTS'], ref_obj['TESTS']
    diff = 0
    count = 0
    for test, results in in_tests.items():
        if args.no_trap and results and results.get('TRAP'):
            continue
        count += 1
        if ref_tests.get(test) == results:
            continue
        if args.print_test or args.print_diff:
            print(f"{test}:")
        if args.print_diff:
            in_state = yaml.dump(results, default_flow_style=False)
            ref_state = yaml.dump(ref_tests.get(test), default_flow_style=False)
            print(f"  REF:")
            for l in ref_state.splitlines():
                print(f"    {l}")  
            print(f"  TST:")
            for l in in_state.splitlines():
                print(f"    {l}")  
        diff += 1
    if diff > 0:
        sys.exit(f"FAILURE: {diff}/{count} differ")
    print(f"PASSED: all {count} tests match")
    return 0

if __name__ == "__main__":
    sys.exit(main())
