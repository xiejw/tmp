## Project Butler

### Motivation

It is essential to have a tool to manage the state of the machine. It can do the
following tasks

1. Set up the machine into desired state. Possible task list includes git,
   packages, networks, etc. Possible action list includes fixing/advancing the
   machine from broken state to desired state. This includes adding new
   packages, removing packages, auto syncing and clean up.

2. One special task which is quite challenging is: Compile some customized
   software (llvm, kernel) and install them.

### History

Two projects have been developed in the past.

- `dotfiles/sync_git_repo` (now in `y`) does simple tasks to go into a bunch of
  configurable git repo (locally) and sync them. However, it does not set up a
  new repo automatically, and it does not report any deprecated repo.

- `hosts` (still private) records all config files (ngix, kernel configs, cron
  tables, etc). But it is running in very passive mode. It records but not
  fixes. Also during set up a new machine, it needs lots of steps to bootstrap
  the new machine.
