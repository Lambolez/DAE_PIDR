#!/opt/csw/bin/perl
#
# Weidong Chen & Bart Lamiroy
# February 2011
#
# This script takes 5 arguments, one of which is optional.
#   - $ARGV[1] the archive file or directory to prepare for upload
#   - $ARGV[2] the destination path to upload the archive to (WARNING! the path argument should *NOT* end with '/')
#   - $ARGV[3] a short description of the archive
#   - $ARGV[4] the DAE database user ID of the person uploading the archive
#   - $ARGV[5] an optional Copyright file applying to all data in the archve.
#
# NOTE: all paths and filenames should be specified with absolute paths.
#
# The script will create the destination path (or fail if it already exists) and move (or extract) the archive to the specified path.
# It will then parse all files and create two output files.
#   - a almost final DAE XML markup file associating all found files with file types, copyright and contributor attributes, as well
#     as a replica of the directory hierarchy using embedded datasets.
#   - a list of all file types encountered, allowing to decide which of those need to be created in the database on final upload
#     and which should be considered for mapping to existing datatypes.
#
use Switch;

my $fileCMD = '/opt/sfw/bin/gfile';
my $identifyCMD = '/opt/csw/bin/identify';

if ($#ARGV < 3)
  {print "command <archive> <path> <desc> <uid> [copyright file]\n";
   exit;}

if ($#ARGV >= 5) {
  ($archive, $dirname, $desc, $uid, $cpfname)=@ARGV;
  if("$cpfname" && ! -e "$cpfname"){die "$cpfname doesn't exist!\n"; }
}
else {($archive, $dirname, $desc, $uid)=@ARGV;}

#make directory for extraction
mkdir($dirname) || die "Cannot create folder! Duplicate foldername '$dirname'!\n";

#extract given archive to given directory

my $fullname=($archive =~ /\//)?($archive =~ /^.*[\/]([^\/]+?$)/)[0]:$archive;
my ($basename, $ext) = split(/\./,$fullname);

# Case where $archive argument is a file (web-based call or manually handled archive)
if (-f $archive) {
   switch ($ext) {
	case "tar" { $archiveArgs = "/opt/csw/bin/gtar xf $archive" }
	case "zip" { $archiveArgs = "unzip $archive" }
	case "rar" { $archiveArgs = "unrar x $archive" }
	case "tgz" { $archiveArgs = "/opt/csw/bin/gtar xzf $archive" }
   }

   chdir($dirname);
   `/usr/bin/bash -c "$archiveArgs"` == 0 || die "$archiveArgs\n Invalid archive file ($ext)! $?\n ";
}
# Case where $archive argument is a directory (manually handled upload)
elsif (-d $archive) {
   chdir($dirname);
   `mv "$archive" "."`;
} else { die "$archive\n Invalid archive argument\n"; }

my $basedirname=($dirname =~ /\//)?($dirname =~ /^.*[\/]([^\/]+?$)/)[0]:$dirname;

#mv copyright file to the new directory if copyright file is not there
$cpfid=($cpfname =~ /\//)?($cpfname =~ /^.*[\/]([^\/]+?$)/)[0]:$cpfname;

if ($cpfname && ! -e "$dirname/$cpfid"){
   my @args1 = ("mv", $cpfname, $dirname);
   system(@args1) == 0 || die "@args1 failed: $?";
   $cpfname = "$basedirname/$cpfid";
}
elsif ($cpfname) {
   my @args2 = ("mv", $cpfname, $dirname."/..");
   system(@args2) == 0 || die "@args2 failed: $?";
   $cpfname = $cpfid;
}

#change dir into the newly created directory - just above in fact
chdir($dirname);
chdir('..');

########parameters for running process_files method
$tab=0;
$sp="\t";
$root=0;
%imgtypes;

##############################

#start printing xml file
#$printout="";
#$printout.= "<metadata>\n";

#process_files($dirname);

#process_files($basedirname);


#$printout = "<metadata>\n" . &mapping . &copyright . $printout;
#$printout.= "</metadata>\n";

open(XML, ">$dirname"."_partialdata.xml") || die "Failed to open xml for writing!\n";

print XML "<metadata>\n";


print XML &mapping;
print XML &copyright;
process_files($basedirname);
#print XML $printout;

print XML "</metadata>\n";
close XML;

open(TYPEFILE, ">$dirname"."_types.txt") || die "Failed to open types for writing!\n";
for(keys %imgtypes)
{
chomp;
print TYPEFILE "image/$_\n";
}
close TYPEFILE;

################################################################
#     END OF MAIN PROGRAM
###############################################################

sub mapping{
$tab++;
my $result="<mapping>\n";

$result.= "$sp"x(1+$tab)."<pair>\n";
$result.= "$sp"x(2+$tab)."<local>person_$uid</local>\n";
$result.= "$sp"x(2+$tab)."<db>$uid</db>\n";
$result.= "$sp"x(1+$tab)."</pair>\n";

$result.="</mapping>\n";

$tab--;
return $result;
}

sub process_files{
  my $path=shift;
  my $datasetid=$path;
  my $name=($path =~ /\//)?($path =~ /^.*[\/]([^\/]+?$)/)[0]:$path;
  opendir (DIR, $path) or die "Unable to open $path: $! (check directory name is not ending with '/')";
  my @files=grep { !/^\.{1,2}$/ } readdir (DIR);
  closedir(DIR);
  @files = map { $path . '/' . $_} @files;

  #@files = sort(@files);

  my $flip=0;
  my $included='';
  
  for (@files){
    if (-d $_){
      process_files($_);
    }
  }
  
  #writing dataset metadate
  $tab++;
  my $datmet="$sp"x$tab."<dataset>\n";
  $tab++;
  $datmet.= "$sp"x$tab."<id>$datasetid</id>\n";
  $datmet.= "$sp"x$tab."<name>$name</name>\n";
  $datmet.= "$sp"x$tab."<description>$desc</description>\n" if ($root++==0);
  $datmet.= &printuid;
  $datmet.= &printcpright;
  $tab--;
  $tab--;
  print XML $datmet;
  
  for (@files){
    if (-d $_){
      $tab++;
      $tab++;
      $datmet = "$sp"x$tab."<associating_dataset_list>\n" if (($flip++)==0);
      $datmet.= "$sp"x($tab+1)."<associating_dataset_list_item>\n";
      $datmet.= "$sp"x($tab+2)."<id>$_</id>\n";
      $datmet.= "$sp"x($tab+1)."</associating_dataset_list_item>\n";      
      
      $tab--;
      $tab--;
      print XML $datmet;
      $datmet = "";
    }
  }
  print XML "$sp"x($tab+2)."</associating_dataset_list>\n" if ($flip>0);
  $tab++;
  print XML "$sp"x$tab."</dataset>\n";
  $tab--;
  for(@files){
	if (!(-d $_)){
		print XML &doimage($_, $datasetid) || &dofile($_, $datasetid);
		#print XML $included;
		#print "included: ".length($included)."\n";
		
	}
  }
  #print "datemet: ".length($datmet)."\n";

}



sub doimage{
  my $f=shift;
  my $datasetid=shift;
  my $ftype=`$fileCMD -b --mime-type '$f'`;
  ($tp, $subtp)=split(/\//,$ftype);
  #print the metadata if this is image
  my $result;
  if ($tp =~ /image/){
    ++$imgtypes{$subtp};
    ($width, $height)=split(/ /,`$identifyCMD -format "%w %h" '$f`);
    chomp $height;
    chomp $subtp;
    $tab++;
    $result.= "$sp"x$tab."<page_image>\n";
    $tab++;
    $result.= "$sp"x$tab."<id>$f</id>\n";
    $result.= "$sp"x$tab."<path>$f</path>\n";
    $result.= "$sp"x$tab."<width>$width</width>\n";
    $result.= "$sp"x$tab."<height>$height</height>\n";
    $result.= "$sp"x$tab."<image_dataset_list>\n";
    $result.= "$sp"x(1+$tab)."<image_dataset_list_item>\n";
    $result.= "$sp"x(2+$tab)."<id>$datasetid</id>\n";
    $result.= "$sp"x(1+$tab)."</image_dataset_list_item>\n";
    $result.= "$sp"x$tab."</image_dataset_list>\n";
    $result.= &printcpright;
    $result.= &printuid;
    $result.= "$sp"x$tab."<type_list>\n";
    $result.= "$sp"x(1+$tab)."<type_list_item>\n";
    $result.= "$sp"x(2+$tab)."<id>$tp/$subtp</id>\n";
    $result.= "$sp"x(2+$tab)."<name>$tp/$subtp</name>\n";
    $result.= "$sp"x(1+$tab)."</type_list_item>\n";
    $result.= "$sp"x$tab."</type_list>\n";
    $tab--;
    $result.= "$sp"x$tab."</page_image>\n";
    $tab--;
    
  }
  return $result;
}


sub printcpright{
  my $result;
  if($cpfname){    
    $result.="$sp"x$tab."<copyright_list>\n";
    $result.="$sp"x(1+$tab)."<copyright_list_item>\n";
    $result.="$sp"x(2+$tab)."<id>$cpfid</id>\n";
    $result.="$sp"x(1+$tab)."</copyright_list_item>\n";
    $result.="$sp"x$tab."</copyright_list>\n";
  }
  return $result;
}

sub printuid{
  my $result;
  $result.= "$sp"x$tab."<contributor_list>\n";
  $result.= "$sp"x($tab+1)."<contributor_list_item>\n";
  $result.= "$sp"x($tab+2)."<person_id>person_$uid</person_id>\n";
  $result.= "$sp"x($tab+1)."</contributor_list_item>\n";
  $result.= "$sp"x$tab."</contributor_list>\n";
  return $result;
}

sub copyright{

  if($cpfname) {
    my $f=$cpfname;
    my $filename=($f =~ /\//)?($f =~ /^.*[\/]([^\/]+?$)/)[0]:$f;
    
    $result.= "$sp"x$tab."<copyright>\n";
    $tab++;
    $result.= "$sp"x$tab."<id>$cpfid</id>\n";
    $result.= "$sp"x$tab."<name>$filename</name>\n";
    $result.= "$sp"x$tab."<path>$f</path>\n";
    $result.= &printuid;

    $tab--;
    $result.= "$sp"x$tab."</copyright>\n";
  }
  return $result;
}

sub dofile{
  my $f=shift;
  my $datasetid=shift;
  my $filename=($f =~ /\//)?($f =~ /^.*[\/]([^\/]+?$)/)[0]:$f;
  my $name1=($f =~ /^.*[\/]([^\/]+?$)/)[0];
  #my $iscpright= ($cpfname eq $name1);
  my $result;
  $tab++;

  #if(!$iscpright){
    $result.= "$sp"x$tab."<file>\n";
  
    $tab++;
    $result.= "$sp"x$tab."<id>$f</id>\n";
    $result.= "$sp"x$tab."<name>$filename</name>\n";
    $result.= "$sp"x$tab."<path>$f</path>\n";
    $result.= "$sp"x$tab."<dataset_list>\n";
    $result.= "$sp"x(1+$tab)."<dataset_list_item>\n";
    $result.= "$sp"x(2+$tab)."<id>$datasetid</id>\n";
    $result.= "$sp"x(1+$tab)."</dataset_list_item>\n";
    $result.= "$sp"x$tab."</dataset_list>\n";
    $result.= &printcpright;
    $result.= &printuid;

    $tab--;
    $result.= "$sp"x$tab."</file>\n";
  #}
  $tab--;
  return $result;
  
 
}



