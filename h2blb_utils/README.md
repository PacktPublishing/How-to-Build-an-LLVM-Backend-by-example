This directory contains:
- tag2commitmsg.txt: the list of tags that need to be maintained to map to the content of the companion book.
- retag.py: a script that look for the messages that matches what is tagged and reapply the tags

This is useful to re-apply the tags when rebasing LLVM for instance.

The script assumes that:
- It is run from the root directory of LLVM, and
- Each commit message uniquely identify the tag(s) we are looking for

To use the script, simply run from the root directory:
```bash
python h2blb_utils/retag.py h2blb_utils/tag2commitmsg.txt
```

The `tag2commitmsg.txt` can be regenerated by running `git log --oneline begin_ch9^..HEAD` before a rebase and copying the output without the first column (i.e., the git hash needs to be omitted.)

More specifically one can use (assuming `origin` has the proper tags):
```bash
git log --oneline --decorate=short origin/main begin_ch9^..HEAD 2>&1 | grep 'tag:' | cut -f 2- -d ' '
```

Essentially this command:
- `git log` part: lists all the commits since the first tag (`begin_ch9`) and decorate the output with the tag names
- `grep` part: keeps only the commits with a tag
- `cut` part: keeps only the field (`f`) starting from the second one (`2-`), where the fields are delimited by a space (`-d ' '`), i.e., we get rid of the git hash that is printed first in the `oneline` output
