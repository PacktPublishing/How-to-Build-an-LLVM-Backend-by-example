# Map a commit msg to a list of tags.
# Keep looking for commits until max search is hit or all the commits were found.
# Print out the commits that couldn't be found.

import argparse
import os
import re
import subprocess

debug = 2
local_only = True

def debug_print(level, *args, **kwargs):
  if level <= debug:
    print(*args, **kwargs)

def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Take a file with a mapping of tags to commit messages and reapply the tags.")

  parser.add_argument("filename", help="The name of the file with the mapping", type=str)
  parser.add_argument("-c", "--chunk", help="How many commits to look at at a time", default=100, type=int)
  parser.add_argument("-i", "--iteration", help="How many iteration of lookup are made. Put differently this and the chunk options determine the maximum amount of commits that are scanned", default=10, type=int)
  parser.add_argument("-d", "--debug-level", help="How much printing should we do for debugging purpose: 0 (none) -> 5 (all)", default=debug, type=int)
  parser.add_argument("-l", "--local-only", help="Update the tag only locally. I.e., do not push the changes to the server", default=local_only, type=bool)

  args = parser.parse_args()
  return args

def validate_args(args: argparse.Namespace) -> (bool, str):
  succeeded = True
  msg = ""
  global debug
  debug = args.debug_level
  global local_only
  local_only = args.local_only
  if not os.access(args.filename, os.R_OK):
    succeeded = False
    msg = args.filename + " is not readable\n"
  if args.chunk <= 0:
    succeeded = False
    msg = f"chunk must be greater than 0, got: {args.chunk}\n"
  if args.iteration <= 0:
    succeeded = False
    msg = f"iteration must be greater than 0, got: {args.iteration}\n"
  return succeeded, msg

def build_msg2tags_dict(mapping_file: str) -> (dict, set):
  tags = set()
  msg2tags = {}
  with open(mapping_file, "r") as file:
    for line in file:
      # Ignore lines that don't start with a tag.
      match = re.search(r"^\((.*)\) (.*)$", line)
      if match:
        tags_str = match.group(1)
        msg_str = match.group(2)
        tags_list = tags_str.split(',')
        cleaned_tags = set()
        for tag in tags_list:
          tag = re.sub(r" *tag: *", "", tag)
          tags.add(tag)
          cleaned_tags.add(tag)
        msg2tags[msg_str.strip()] = cleaned_tags
      else:
        debug_print(1, f"discard: {line}")
  return msg2tags, tags

def update_tag(tag: str, git_hash: str):
  debug_print(1, f"Update tag '{tag}' at '{git_hash}'")
  global local_only

  cmds = []
  # Delete the tag locally.
  cmds.append(f"git tag -d {tag}")
  # Delete the tag on the server.
  if not local_only:
    cmds.append(f"git push --delete origin tag {tag}")
  # Apply the tag locally.
  cmds.append(f"git tag {tag} {git_hash}")
  # Push the tag on the server.
  if not local_only:
    cmds.append(f"git push origin tag {tag}")
  for cmd in cmds:
    debug_print(2, cmd)
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if result.returncode != 0:
      print(f"error running the following command: 'cmd'")
      exit(-3)

def main():
  args = parse_args()
  succeed, msg = validate_args(args)
  if not succeed:
    print(f"error: {msg}\n")
    exit(-1)
  msg2tags, tags = build_msg2tags_dict(args.filename)
  for key, value in msg2tags.items():
    debug_print(3, f"key {key}")
    debug_print(3, f"values {value}")
  print(f"Nb tags to find {len(tags)}")

  cur_iteration = 0
  skip = 0
  while tags and cur_iteration != args.iteration:
    cur_iteration += 1
    print(f"Iteration {cur_iteration}: Looking at next {args.chunk} commits")
    cmd = f"git log --oneline -n {args.chunk} --skip {skip}"
    debug_print(2, cmd)
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if result.returncode != 0:
      print(f"error the following cmd failed: {cmd}")
      break
    for commit in result.stdout.split('\n'):
      match = re.search(r"^([^ ]+) +(\([^)]+\))? *(.*)$", commit)
      if match:
        git_hash = match.group(1)
        commit_tags = match.group(2)
        commit_msg = match.group(3).strip()
        debug_print(4, f"hash: {git_hash}")
        debug_print(4, f"tags: {commit_tags}")
        debug_print(4, f"commit: {commit_msg}")
        if commit_msg in msg2tags:
          set_of_tags = msg2tags[commit_msg]
          for tag in set_of_tags:
            debug_print(2, f"found tag '{tag}' at '{git_hash}'")
            tags.remove(tag)
            update_tag(tag, git_hash)
          # stop processing the commits if we already found everything
          if not tags:
            break
      elif commit != '':
        debug_print(1, f"no match: {commit}")
    print(f"Nb tags to find: {len(tags)}")
    skip += args.chunk

  if tags:
    print(f"Didn't find the following {len(tags)} tags: {tags}")
    exit(-2)
  exit(0)

if __name__ == "__main__":
  main()
