#!/bin/bash

tmp=$(mktemp)

jq '.username="'$1'" ' /etc/gw_web_admin/database.json > "$tmp" && mv "$tmp" /etc/gw_web_admin/database.json
jq '.password="'$2'" ' /etc/gw_web_admin/database.json > "$tmp" && mv "$tmp" /etc/gw_web_admin/database.json

sudo chmod +r /etc/gw_web_admin/database.json
