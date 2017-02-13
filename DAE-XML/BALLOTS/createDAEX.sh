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

urlDAE='http://dae.cse.lehigh.edu/DAE-SBX'

if [ $# -ne 3 ] ; then
   echo "`basename $0`: usage `basename $0` AnnotationDir BallotName AnnotatorName";
   exit 1;
fi

execdir=`dirname "$0"`;
ballot=$2;
ballotdirname="$1";
annotatorName="$3";

#
# Defining used constant strings for datatype values
#
datatype[0]="Ballot Annotation Area";
datatype[1]="Description String";
datatype[2]="Numeric Identifier";
datatype[3]="Alphanumeric Identifier";
datatype[4]="boolean";

datatypeProperty[0]="Ballot Race Name";
datatypeProperty[1]="Ballot Race Identifier";
datatypeProperty[2]="Ballot Candidate Identifier";
datatypeProperty[3]="Ballot Candidate Name";
datatypeProperty[4]="Ballot Vote Legality";
datatypeProperty[5]="Ballot Vote Markup Type";
datatypeProperty[6]="Ballot Annotator";

#
# First Phase:
# Retrieving pre-defined identifiers from DAE database
#
ballotID=`wget -q -O - "${urlDAE}/?q=Repository/query/page_image/path/${ballot}"`;

declare -A datatypeId
for i in "${datatype[@]}"
do
  datatypeId[$i]=`wget -q -O - "${urlDAE}/?q=Repository/query/datatype/name/$i"`;
done

declare -A datatypePropertyId
for i in "${datatypeProperty[@]}"
do
  datatypePropertyId[$i]=`wget -q -O - "${urlDAE}/?q=Repository/query/datatype_property/name/$i"`;
done

#
# Some hardwired names ... should be fixed one day
#
datatypeProperty[7]="fiducial";
datatypePropertyId["fiducial"]=`wget -q -O - "${urlDAE}/?q=Repository/query/datatype_property/name/Ballot Fiducial Mark"`;

datatypeProperty[8]="handwriting_official";
datatypePropertyId["handwriting_official"]=`wget -q -O - "${urlDAE}/?q=Repository/query/datatype_property/name/Ballot Official Hand Writing"`;

datatypeProperty[9]="handwriting_voter";
datatypePropertyId["handwriting_voter"]="";

datatypeProperty[10]="handwriting";
datatypePropertyId["handwriting"]="";

datatypeProperty[11]="handwriting_voter";
datatypePropertyId["handwriting_voter"]="";

datatypeProperty[12]="stamp";
datatypePropertyId["stamp"]="";

datatypeProperty[13]="other";
datatypePropertyId["other"]="";

datatypeProperty[14]="stray";
datatypePropertyId["stray"]="";

datatypeProperty[15]="label";
datatypePropertyId["label"]="";

datatypeProperty[16]="skew";
datatypePropertyId["skew"]=`wget -q -O - "${urlDAE}/?q=Repository/query/datatype_property/name/Ballot Skew Line"`;


#
# Second Phase: generating the mapping information
#
#
echo "
<metadata>
  <mapping>
   <pair>
      <!-- page_image -->
      <local>${ballot}</local>
      <db>${ballotID}</db>
   </pair>"

for i in "${datatype[@]}"
do
  echo "   
   <pair>
      <!-- Datatype -->
      <local>$i</local>
      <db>${datatypeId[$i]}</db>
   </pair>"
done

for i in "${datatypeProperty[@]}"
do
  echo "   
   <pair>
      <!-- Datatype Property -->
      <local>$i</local>
      <db>${datatypePropertyId[$i]}</db>
   </pair>"
done


echo "
</mapping>

<page_image>
  <id>${ballot}</id>
  <page_element_list>
"

#
# Third Phase: parse the .tru file to get the information not available in the XML file
#

value=0;
modstring="";

while read line
do

value=`expr $value + 1`;
type=`echo $line | awk '{print $1}'`;
x1=`echo $line | awk '{print $2}' | sed 's/^\([1234567890]\+\),.\+/\1/'`;
y1=`echo $line | awk '{print $2}' | sed 's/^.\+,\([1234567890]\+\)/\1/'`;
x2=`echo $line | awk '{print $3}' | sed 's/^\([1234567890]\+\),.\+/\1/'`;
y2=`echo $line | awk '{print $3}' | sed 's/^.\+,\([1234567890]\+\)/\1/'`;
suppl=`echo $line | awk '{print $4}'`;

echo "$x1,$y1 - $x2,$y2." >&2

if [ $type = "valid" ] || [ $type = "cancelled" ] ; then

   modstring="$modstring -e s/${suppl}_X/$x1/ -e s/${suppl}_Y/$y1/ -e s/${suppl}_W/`expr $x2 - $x1`/ -e s/${suppl}_H/`expr $y2 - $y1`/";

else

echo "
<page_element_list_item>
  <id>${ballot}_${value}</id>
  <description>Ballot Annotation Area</description>
  <topleftx>$x1</topleftx>
  <toplefty>$y1</toplefty>
  <width>"`expr $x2 - $x1`"</width>
  <height>"`expr $y2 - $y1`"</height>
  <type_list>
    <type_list_item>
      <id>Ballot Annotation Area</id>
    </type_list_item>
  </type_list>
  <value_list>
    <value_list_item>
      <id>${ballot}_${value}_1</id>
      <description></description>
      <value>$suppl</value>
      <property>
        <id>$type</id>
      </property>
    </value_list_item>
    <value_list_item>
      <id>${ballot}_${value}_Annotator_1</id>
      <description></description>
      <value>${annotatorName}</value>
      <property>
        <id>Ballot Annotator</id>
      </property>
    </value_list_item>
  </value_list>
</page_element_list_item>
"


fi

done < "${ballotdirname}/${ballot}.tru"

#
# Fourth Phase: transform the XML data stored in the annotation directory
#

xsltproc "${execdir}/transform.xsl" "${ballotdirname}/${ballot}.xml" | tail --lines=+2 | sed $modstring -e "s|<value>BallotAnnotatorNameTag</value>|<value>${annotatorName}</value>|"



echo "
</page_element_list>
</page_image>
</metadata>"
