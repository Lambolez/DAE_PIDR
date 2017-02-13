#!/bin/bash
# 
# Bart Lamiroy, April 2011
#
# V0.1
#
# Assumes the presence of a .tru and an .xml file in the directory passed as arg1
#
# run fromdos
# Correct XML closing tags in .note
# Correct tag naming: sed -e 's|\(<radiobutton .*\)>|\1/>|' -e 's|\(<checkbutton .*\)>|\1/>|' -e 's|\(<text .*\)>|\1/>|' -e 's|\(<entry .*\)>|\1/>|' -e 's/<race\([1234567890]\+\) /<race id="\1" /' -e 's_</race[1234567890]\+>_</race>_' -e 's/<cand\([^ ]\+\) /<cand id="\1" /' -e 's_</cand[^ ]\+>_</cand>_' #
#

urlDAE='http://dae-nlm.cse.lehigh.edu/DAE'

if [ $# -ne 1 ] ; then
   echo "`basename $0`: usage `basename $0` DocumentName";
   exit 1;
fi

execdir=`dirname "$0"`;
document=$1;

documentDirName=`dirname "$document"`;
documentExtension=.${document/*./};
documentFileName=`basename "$document" "$documentExtension"`;
documentName=`basename "$documentDirName"`_"$documentFileName";
documentHTMLName=`basename "$documentDirName"`%25$documentFileName

#
# Defining used constant strings for datatype values
#
declare -A datatypeId
datatype[0]="MARG Zone";
datatypeId["MARG Zone"]='2076';
datatype[1]="MARG Line";
datatypeId["MARG Line"]='2077';
datatype[2]="MARG Word";
datatypeId["MARG Word"]='2078';
datatype[3]="MARG Character";
datatypeId["MARG Character"]='2079';

declare -A datatypePropertyId
datatypeProperty[0]="Page Element Inclusion";
datatypePropertyId["Page Element Inclusion"]=1041
datatypeProperty[1]="MARG Classification Category";
datatypePropertyId["MARG Classification Category"]=1042
datatypeProperty[2]="MARG Classification Type";
datatypePropertyId["MARG Classification Type"]=1043
datatypeProperty[3]="MARG PointSize";
datatypePropertyId["MARG PointSize"]=1044
datatypeProperty[4]="MARG Style";
datatypePropertyId["MARG Style"]=1045
datatypeProperty[5]="MARG Color";
datatypePropertyId["MARG Color"]=1046
datatypeProperty[6]="MARG Next Zone";
datatypePropertyId["MARG Next Zone"]=1047
datatypeProperty[7]="MARG Next Line";
datatypePropertyId["MARG Next Line"]=1048
datatypeProperty[8]="MARG Next Word";
datatypePropertyId["MARG Next Word"]=1049
datatypeProperty[9]="MARG Next Character";
datatypePropertyId["MARG Next Character"]=1050
datatypeProperty[10]="MARG Text";
datatypePropertyId["MARG Text"]=1051


#
# First Phase:
# Retrieving pre-defined identifiers from DAE database
#
documentID=`wget -q -O - "${urlDAE}/?q=Repository/query/page_image/path/${documentHTMLName}"`;

for i in "${datatype[@]}"
do
  if [ ! ${datatypeId[$i]} ] ; then datatypeId[$i]=`wget -q -O - "${urlDAE}/?q=Repository/query/datatype/name/$i"`; fi
done

for i in "${datatypeProperty[@]}"
do
  if [ ! ${datatypePropertyId[$i]} ] ; then datatypePropertyId[$i]=`wget -q -O - "${urlDAE}/?q=Repository/query/datatype_property/name/$i"`; fi
done


#
# Second Phase: generating the mapping information
#
#
modifierString="";

for i in "${datatype[@]}"
do
  modifierString="${modifierString} -e s#<db>${i//\ /_}</db>#<db>${datatypeId[$i]}</db>#"
done

for i in "${datatypeProperty[@]}"
do
  modifierString="${modifierString} -e s#<db>${i//\ /_}</db>#<db>${datatypePropertyId[$i]}</db>#"
done

#
# Fourth Phase: transform the XML data stored in the annotation directory
#

xsltproc --path "${execdir}" --param "imagename" "'$documentName'" --param "imagepath" "'$document'" --param "imageid" "'${documentID}'" "${execdir}/transform.xsl" "${documentDirName}/${documentFileName}.xml" | tail -n +2 | sed ${modifierString} 


