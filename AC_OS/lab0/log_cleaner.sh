    #!/bin/bash

# Лабораторная_1 Скрипт для очистки
# Патык Дмитрий, Шошинов Николай 24КНТ2

#Helping
show_help() {
    echo "Usage: $0 [DIR] [BACKUP_DIR] [X]"
    echo ""
    echo "DIR = directory to compress"
    echo "BACKUP_DIR = directory to save archive"
    echo "X = occupancy percentage to start compress (default: 70)"
    echo ""
    echo "Example: $0 /log /backup 70"
}

#Function for % of disk
get_disk_percent() {
	local dir="$1"
    usage=$(df "$dir" | awk 'NR==2 {print $5}' | sed 's/%//')
    echo "$usage"
}

files_to_archive() {
    local dir="$1"
    local Xhold="$2"

    local current_dir=$(pwd)

    current_used_space=$(df -B1 "$dir" | awk 'NR==2 {print $3}')
    max_space=$(df -B1 "$dir" | awk 'NR==2 {print $2}')

    current_usage=$(get_disk_percent "$dir")

    if [ "$current_usage" -le "$Xhold" ]; then
        echo "0"
        return 0
    fi

    cd "$dir"

    files_list=$(ls -1t)
    number_of_files=0

    temp_usage="$current_usage"
    for file in $files_list; do

        if [ ! -f "$file" ]; then
            continue
        fi

        temp_usage=$(( current_used_space * 100 / max_space ))
        if [ "$temp_usage" -le "$Xhold" ]; then
            break
        fi

        ((number_of_files++))
        current_used_space=$(( current_used_space - $(stat -c%s "$file") ))

    done

    cd "$current_dir"

    echo "$number_of_files"
}

archive_files() {
    local source_dir="$1"
    local backup_dir="$2"
    local Xhold="$3"

    n_files=$(files_to_archive "$source_dir" "$Xhold")

    cd "$source_dir" || { echo "Cannot enter directory $source_dir"; return 1; }

    echo "Looking for $n_files the most old files in $source_dir..."

    time_creating=$(date +"%Y%m%d_%H%M%S")
    if [ "$LAB1_MAX_COMPRESSION" == 1 ]; then
        archive_name="backup_${time_creating}.tar.xz"
    else
        archive_name="backup_${time_creating}.tar.gz"
    fi
    archive_path="../$backup_dir/$archive_name"

    files_archive=$(ls -1t | tail -n "$n_files")

    if [ -z "$files_archive" ]; then
        echo "No files found to archive"
        return 1
    fi

    echo "Files to archive:"
    echo "$files_archive"

    echo "Creating archive: $archive_path"

    if [ "$LAB1_MAX_COMPRESSION" == 1 ]; then
        tar --lzma -cf "$archive_path" $files_archive
        echo "Archived via LZMA!"
    else
        tar -czf "$archive_path" $files_archive
    fi


    if [ $? -eq 0 ]; then
        echo "Archive created successfully: $archive_name"
        echo "Removing archived files..."
        rm $files_archive
        echo "Files removed"
        return 0
    else
        echo "Error: Failed to create archive"
        return 1
    fi
}

#Input_Helping
if [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    show_help
    exit 0
fi

#Input_check
if [ $# -lt 2 ]; then
    echo "Error: You need to enter directory and backup directory as parameters."
    echo "Use -h or --help for help"
    exit 1
fi

#Input parameters
directory=$1
backupdir=$2
X=${3:-70}

#Script
echo "=== Script started ==="
echo "Directory to monitor: $directory"
echo "Backup directory: $backupdir"
echo "Occupancy threshold: $X%"
echo ""

#Check dirs
if [ ! -d "$directory" ]; then
    echo "Error: Directory $directory does not exist!"
    exit 1
fi

if [ ! -d "$backupdir" ]; then
    echo "Creating backup directory: $backupdir"
    mkdir -p "$backupdir" || { echo "Error: Cannot create backup directory"; exit 1; }
fi

#Check disk usage
echo "Checking disk usage..."
current_usage=$(get_disk_percent "$directory")
echo "Current disk usage: $current_usage%"

#Compare with threshold >=
if [ "$current_usage" -ge "$X" ]; then
    echo "Threshold exceeded! Starting archiving process..."

    archive_files "$directory" "$backupdir" "$X"
    if [ $? -eq 0 ]; then
        echo "Cleanup completed successfully"

	if [ -d "$directory" ]; then
	    new_usage=$(get_disk_percent "$directory")
	    echo "New disk usage: $new_usage%"
	else
	    echo "Directory removed during cleanup"

fi
    else
        echo "Cleanup failed"
        exit 1
    fi
else
    echo "Disk usage is below threshold. No action needed."
fi

echo "=== Script finished ==="
