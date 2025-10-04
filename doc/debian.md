https://www.debian.org/doc/debian-policy/ch-binary.html#s3.7

```
$ dpkg-query -Wf '${Package;-40}${Essential}\n' | grep yes
base-files                              yes
base-passwd                             yes
bash                                    yes
bsdutils                                yes
coreutils                               yes
dash                                    yes
debianutils                             yes
diffutils                               yes
dpkg                                    yes
findutils                               yes
grep                                    yes
gzip                                    yes
hostname                                yes
init-system-helpers                     yes
libc-bin                                yes
login                                   yes
ncurses-base                            yes
ncurses-bin                             yes
perl-base                               yes
sed                                     yes
sysvinit-utils                          yes
tar                                     yes
util-linux                              yes
```

```
dpkg-query -Wf '${Package;-40}${Priority}\n' | grep -E "required"
apt                                     required
base-files                              required
base-passwd                             required
bash                                    required
bsdutils                                required
coreutils                               required
dash                                    required
debconf                                 required
debianutils                             required
diffutils                               required
dpkg                                    required
e2fsprogs                               required
findutils                               required
grep                                    required
gzip                                    required
hostname                                required
init-system-helpers                     required
libc-bin                                required
libpam-modules                          required
libpam-modules-bin                      required
libpam-runtime                          required
login                                   required
mawk                                    required
mount                                   required
ncurses-base                            required
ncurses-bin                             required
passwd                                  required
perl-base                               required
sed                                     required
sysvinit-utils                          required
tar                                     required
tzdata                                  required
util-linux                              required
zlib1g                                  required
```

```
$ dpkg-query -Wf '${Package;-40}${Priority}\n' | grep -E "important"
adduser                                 important
apt-utils                               important
cpio                                    important
cron                                    important
cron-daemon-common                      important
debconf-i18n                            important
debian-archive-keyring                  important
dmidecode                               important
fdisk                                   important
gpgv                                    important
groff-base                              important
ifupdown                                important
init                                    important
iproute2                                important
iputils-ping                            important
isc-dhcp-client                         important
isc-dhcp-common                         important
kmod                                    important
less                                    important
logrotate                               important
man-db                                  important
nano                                    important
netbase                                 important
nftables                                important
procps                                  important
readline-common                         important
sensible-utils                          important
systemd                                 important
systemd-sysv                            important
tasksel-data                            important
udev                                    important
vim-common                              important
vim-tiny                                important
whiptail                                important
```
