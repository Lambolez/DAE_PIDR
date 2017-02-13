new_path=""
xslt="gedi.xsl"
temp="./transformed/transformed-"

data_directory=$1
target_metadata=$2
original_metadata=$3
username=$4
password=$5
thumbnails=$6

echo "starting upload process...\n"
sleep 1

echo "removing old temporary xml files..."
rm *.xml

if test -d ./transformed ; then
    echo "clearing ./transformed directory"
    rm transformed/*.xml
else
    mkdir ./transformed
fi
echo "old temporary xml files removed\n"

#recursively looks for all sub-directories of the top level
#directory specified by $1
echo "generating dataset metadata..."
for path in `(find $data_directory -type d -print)`; do
    echo "<dataset>" >> ./dataset.xml
    echo "<id>"$path"</id>" >> ./dataset.xml
    echo "<name>dataset: "$path"</name>" >> ./dataset.xml
    echo "<description>"$path"</description>" >> ./dataset.xml

    echo "<associating_dataset_list>" >> ./dataset.xml
    for subpath in `(find $path -type d -print)`; do
        if [ $path != $subpath ]; then
            echo "<associating_dataset_list_item>" >> ./dataset.xml
            echo "<id>"$subpath"</id>" >> ./dataset.xml
            echo "</associating_dataset_list_item>" >> ./dataset.xml
        fi
    done

    echo "</associating_dataset_list>" >> ./dataset.xml
    echo "</dataset>" >> ./dataset.xml
done
echo "dataset metadata generated\n"

echo "starting transforming image metadata..."
for path in `(find $original_metadata -type f -print)`; do
    new_path=$temp`basename $path`
    echo $new_path
    echo "\n" >> $path
    sed 's/xmlns="http:\/\/lamp.cfar.umd.edu\/GEDI"//g' $path | xsltproc -o temp1.xml $xslt -
    sed -n -e '3,$p' temp1.xml > temp2.xml
    sed '$d' temp2.xml > $new_path
done

echo "<metadata>" >> $target_metadata
cat ./dataset.xml ./transformed/*.xml >> $target_metadata
echo "</metadata>\c" >> $target_metadata

echo "image metadata transformed\n"

java -jar /home/des308/dae/program/oracle/daetoy/upload.jar --username $username --password $password --datapath $data_directory --metapath $target_metadata --thumbnails $thumbnails