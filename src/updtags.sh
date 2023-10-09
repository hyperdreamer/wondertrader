#!/bin/bash

bd_time=$(find build_all/ -type f -printf '%T@\t%p\n' | sort -rn | cut -d. -f1 | head -n 1)

if [ -e tags ]; then 
    tag_time=$(find tags -type f -printf '%T@\t%p\n' | sort -rn | cut -d. -f1 | head -n 1)
fi

if [ ! -e tags ] || [ "$bd_time" -gt "$tag_time" ]; then
    ctags --c-kinds=+p --c++-kinds=+p --fields=+iaS --extras=+qf -R
    echo "tags updated!"
fi

