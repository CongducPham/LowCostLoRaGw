#!/bin/bash

sshfs -o nonempty -o idmap=user -o allow_root cpham@$1:/Users/cpham/Dropbox ~/Dropbox
