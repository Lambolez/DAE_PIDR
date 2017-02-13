#! /bin/bash
#
# Bart.Lamiroy@loria.fr		Jul 18 2012
#
# This script requires 'curl' and 'mktemp' to be installed
#
# The script requires 4 parameters:
#  $1 - username
#  $2 - password
#  $3 - DAE URL
#  $4 - filename
#
# It will use the credentials to connect to the provided
# DAE instance and upload the filename (which is needs to conform
# to the DAE XML markup) 
#

if [ $# -ne 4 ] ; then
   echo "`basename $0`: usage `basename $0` username password URL filename";
   exit 1;
fi

DAEUSER=$1 		#'dae'
DAEPASS=$2 		#'dae'
INSTANCE=$3 	#'http://localhost/localDAE'
FILENAME=$4 	#'test.xml'


COOKIES=`mktemp`
TMPFILE=`mktemp`
#
# Sending Log in Credentials (and storing them in cookies)
#
if ! curl ${INSTANCE}/?q=user \
        -s \
        -c ${COOKIES} \
        -b ${COOKIES} \
        -F "name=${DAEUSER}" \
        -F "pass=${DAEPASS}" \
        -F "form_id=user_login" \
        -F "op=Log in" \
    	-o ${TMPFILE} ; then
    rm	${TMPFILE} ${COOKIES};
	echo "Failed to connect" && exit $? ;
fi

#
# Retrieving the upload form security token
#
HEADER=`mktemp`

if ! curl ${INSTANCE}/?q=browse/upload_metadata \
		-s -S \
		-c ${COOKIES} \
        -b ${COOKIES} \
		-o ${TMPFILE} \
		-D ${HEADER} ; then
	rm	${TMPFILE} ${HEADER} ${COOKIES};
	echo "Failed to connect" && exit $? ;
fi

if (( `head -1 ${HEADER} | cut -f 2 -d \ ` == 403 )) ; then
	cat ${HEADER} ;
	rm	${TMPFILE} ${HEADER} ${COOKIES};
	echo "Authentication failed" && exit 403 ;
fi 


TOKEN=`grep "edit-dae-metadata-upload-form-token" ${TMPFILE} | \
	sed -e 's/.*edit-dae-metadata-upload-form-token" *value="\([^"]*\)".*/\1/'`

BUILD=`grep "form_build_id" ${TMPFILE} | \
	sed -e 's/.*form_build_id" *id="\([^"]*\)".*/\1/'`

#echo "Retrieved Token: ${TOKEN}"
#echo "Retrieved Build ID: ${BUILD}"

#
# Uploading Annotation File
#
if ! curl ${INSTANCE}/?q=browse/upload_metadata \
		-L -s -S \
        -c ${COOKIES} \
        -b ${COOKIES} \
        -F "files[data_file]=@${FILENAME}" \
		-F 'data_copyright=' \
		-F 'data_description_url=' \
        -F 'op=Upload' \
        -F "form_build_id=${BUILD}" \
        -F "form_token=${TOKEN}" \
        -F 'form_id=dae_metadata_upload' \
        -o ${TMPFILE} \
		-D ${HEADER} ; then
	exit $? ;
fi

OPVALUE=0

if grep -q "Upload Failed" ${TMPFILE} ; then 
	OPVALUE=1; 
	echo "Upload Failed";
fi

#
# Logging out and cleaning up
#
curl ${INSTANCE}/?q=logout \
        -s \
        -c ${COOKIES} \
        -b ${COOKIES} \
    	-o ${TMPFILE}

rm	${TMPFILE} ${HEADER} ${COOKIES}

exit $OPVALUE;

