from datetime import datetime
import pathlib

current_version_num  = "0.0.0"

# this script simply takes the cache file and adds a number to it while also generating version header file for dodoc
print("Updating build number...")

source = open(str(pathlib.Path(__file__).parent.resolve()) + "/IncrementCache.text", "r")

# first off let's get the values from file
values = {}
for line in source:

    split = line.replace("\n", "").split(" ")
    if len(split) != 2 or not split[1].isdigit():
        print("Build increment cache file invalid!")
        quit(-1)

    values[split[0]] = int(split[1])
    print("Found: " + split[1] + " builds for branch: " + split[0])

source.close()

# finding the current branch
branch = ""
with open(str(pathlib.Path(__file__).parent.resolve()) + "/../.git/HEAD", "r") as f: content = f.read().splitlines()

for line in content:
    if line[0:4] == "ref:":
        branch = line.partition("refs/heads/")[2]

if branch == "":
    print("Cannot increment without a git repo!")
    quit(-1)

print("Current branch: " + branch)

# incrementing
if branch in values:
    print("Branch found in cache, incrementing...")
    values[branch] += 1
else:
    print("Branch not found in cache, creating a new record...")
    values[branch] = 1

# summing and caching back
total = sum(values.values())
print("Final summed build number: " + total.__str__())

print("Updating cache...")
cache = open(str(pathlib.Path(__file__).parent.resolve()) + "/IncrementCache.text", "w")

for key, val in values.items():
    cache.write(key + " " + val.__str__() + "\n")

cache.close()

# and finally generating the file
print("Generating incremented source file...")
file = open(str(pathlib.Path(__file__).parent.resolve()) + "/../src/misc/version.zig", "w")
file.writelines([
    "// This file is generated automatically on every build\n"
    f"pub const number       : []const u8 = \"{current_version_num}\";\n"
    f"pub const branch       : []const u8 = \"{branch}\";\n"
    f"pub const branch_build : []const u8 = \"{str(values[branch])}\";\n"
    f"pub const total_build  : []const u8 = \"{total}\";\n"
    f"pub const date_time    : []const u8 = \"{datetime.today().strftime('%Y-%m-%d %H:%M:%S')}\";\n"
])
file.close()

print("Done!")