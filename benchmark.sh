#!/bin/bash

curl -o /dev/null -s -w '%{time_total}s\n' localhost
