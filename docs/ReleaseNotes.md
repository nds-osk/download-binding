# Overview

This is Release Notes of download binding ver2.0.

# 1. Environment

I have tested in the following environment:

| AGL               | Board Name                            |
|-------------------|---------------------------------------|
| Chinook 3.0.5     | Renesas R-Car Starter Kit Pro         |

# 2. Changes

Add the following functions:

(The following functions supported only in commercial version.)
- manages files retention period per binder.
- keeps the download information per session, when system rebooted.


# 3. Notices

There are the following known defects:

- Sometimes the download information files are empty when you exit the binder during downloading.

# 4. Restrictions

The following functions described in the release notes are not implemented yet:

- limits number of downloading in parallel in the system.
- limits total save size per binder.
- enables/disables HTTP redirect.