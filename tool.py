#! /usr/bin/python3
import argparse
import re
import os
import sys

parser = argparse.ArgumentParser(description="some practicals functions for test")

parser.add_argument("--file_range", default="" ,type=str, help="fileNo range")
parser.add_argument("-d", default="./", type=str, help="target directory")


def get_file_range(range:str, dir:str):
    res:list[str] = []
    grp = re.match(r"(\d*)-(\d*)", range)
    start, end = map(int, [
        grp[1] if grp[1] != "" else "0",
        grp[2] if grp[2] != "" else "9999"
        ])
    files = os.listdir(dir)
    for file in files:
        if start <= int(re.match(r"(\d+).*", file)[1]) < end:
            res.append(f'{dir}/{file}')
    return res


if __name__ == "__main__":
    args = parser.parse_args()
    directory = args.d
    if args.file_range != "":
        files = get_file_range(args.file_range, directory)
        for file in files:
            print(file)