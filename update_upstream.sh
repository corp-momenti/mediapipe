#!/bin/bash

# If you haeve no upstream, please add it the following command:
# git remote add upstream https://github.com/google/mediapipe.git
git fetch upstream
git rebase upstream/master
git push origin --tags

