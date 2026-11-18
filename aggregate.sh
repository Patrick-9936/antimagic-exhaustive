#!/bin/sh
# sum shard outputs
awk '{for(i=1;i<=NF;i++){split($i,a,"="); if(a[1]=="connected")c+=a[2]; if(a[1]=="antimagic")m+=a[2]; if(a[1]=="undecided")u+=a[2]; if(a[1]=="labelings")l+=a[2]}} END{printf "connected=%d antimagic=%d undecided=%d labelings=%d\n",c,m,u,l}' "$@"
