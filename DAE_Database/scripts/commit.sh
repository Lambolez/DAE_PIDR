#! /bin/bash -ex
#
# Bart Lamiroy (lamiroy@cse.lehigh.edu)
# February 2011
#
# This script uploads a pre-processed directory structure and its associated metatdata.xml file
# into the DAE Oracle database.
#
# It requires 4 input parameters:
#  - the directory provided as an absolute path
#  - the oracle server hostname
#  - the oracle database SID
#  - the email addres of the uploader
#
# It also makes the following assumptions:
#  - $directory is a valid absolute path.
#  - the parent directory of $directory is writeable
#  - Besides $directory, there is a file named $directory_metadata.xml
#  - the directory containing the current script also contains a configuration file (config.txt)
#    containing the credentials to access the oracle database.
#  - the directory containing the current script also contains a .jar file (upload.jar)
#    containing all code to upload data into the oracle database.
#
# ATTENTION! Be aware this script is run with the +e option (will not halt on errors)
#

set +e

cleanUp () {
  # Cleaning up uploaded temporary files
  cd `dirname "${directory}"` 
  rm -rf "${directory}" "${directory}_workdir" "${directory}_metadata.xml" "${directory}_partialdata.xml" "${directory}_types.txt"
}

sendFailure () {
    subject="*FAILURE* Upload for `basename "$directory"` on $url"
    message="
Sorry for this failure.

All uploaded data has been removed.
You may try to upload the data again, or contact the site administrator for more detaled information.";

    echo "$message" | mailx -s "$subject" -b $adminMail $email >> "$logfile" 2>&1
    returnValue=$?

    if [ $returnValue -ne 0 ] ; then
      echo "Failed to send message mailx -s \"$subject\" -b $adminMail $email" >> "$logfile"
    fi

    cleanUp

    exit 1;
}

sendSuccess () {
    subject="*SUCCESS* Upload for `basename "$directory"` on $url"
    message="
Congratulations !

Your uploaded data has been correctly integrated in $url.
Please check whether the upload is available as intended, and contact the site administrator in case of inconsistencies.";

    cleanUp

    echo "$message" | mailx -s "$subject" -b $adminMail $email >> "$logfile" 2>&1
    returnValue=$?

    if [ $returnValue -ne 0 ] ; then
      echo "Failed to send message mailx -s \"$subject\" -b $adminMail $email" >> "$logfile";
    fi

    cleanUp

    exit $returnValue;
}


execDir=`dirname "$0"`
adminMail="dae-admin@dae.cse.lehigh.edu"
prodURL="http://dae.cse.lehigh.edu/DAE"
testURL="http://dae-websbx.cse.lehigh.edu/DAE"

if [ $# -ne 4 ] ; then
   echo "Usage: $0 directory server SID email" && sendFailure
fi

directory=$1
server=$2
SID=$3
email=$4

# Creating a logfile
logfile=`dirname "${directory}"`/`date '+%Y%m%d%H%M%S'`."$RANDOM"_upload.log
date > "$logfile"
echo "$*" >> "$logfile"

# Create message to send upon completion
case $SID in
        dae0 )
            url="${prodURL}"
            ;;
        daetoy )
            url="${testURL}"
            ;;
        *   )
            cat "error: $SID is not a correct database SID." >> "$logfile"
            sendFailure ;;
    esac


# Creating temporary workspace
mkdir "${directory}_workdir" && chmod 700 "${directory}_workdir"

if [ $? -ne 0 ] ; then
   echo "Cannot create temporary work directory" >> "$logfile" && sendFailure
fi

# Configuring credentials
cd "${directory}_workdir"
sed -e s/SID/$SID/ -e s/SERVER/$server/ "$execDir/config.txt" > config.txt

if [ $? -ne 0 ] ; then
   echo "Invalid configuration file 'config.txt'" >> "$logfile" && sendFailure
fi

# Committing all data to Database
java -jar "$execDir/upload.jar" --username bal409@lehigh.edu --password '000000' --datapath "${directory}" --metapath "${directory}_metadata.xml" --thumbnails true >> "$logfile" 2>&1 || sendFailure

sendSuccess


