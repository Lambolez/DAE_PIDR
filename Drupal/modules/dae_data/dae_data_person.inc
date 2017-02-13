<?php
// $Id:$

/**
 * @file dae_data_person.inc
 *
 * @author  Xingjian Zhang <xiz307@lehigh.edu>
 *
 * @version 1.0
 *
 * \brief Provides the class to query and display user's information
 *
 * @section LICENSE
 *
 *  This file is part of the DAE Platform Project (DAE).
 *
 *  DAE is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DAE is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DAE.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

/**
 * This function is called when ?q=person is requested.  It takes the
 * person id from argument(1), fetches information from database and
 * output a page for displaying such information
 *
 * @param $input
 * @return the table (string) that contains the information about the person.
 */
function dae_view_submitter()
{
    $pid=arg(1);
    $person=new Person($pid);
    return $person->getHTMLOutputStr('html');
}

/**
 * This function takes Drupal user id (uid) as input, and returns
 *  the corresponding person id (pid) in the Person table in the Oracle database
 *
 * @param $uid
 * @return the corresponding person id (pid) in the Person table in the Oracle database.
 */
function getPID($uid){
    $sql="SELECT pv.value FROM profile_fields pf, profile_values pv WHERE pf.name = 'profile_person_id' AND pf.fid = pv.fid AND pv.uid =$uid";
    $pid = db_result(db_query($sql));
    if(empty($pid)){ // dealing with the old data of person which don't have a uid-pid mapping by assuming uid=pid.
        /** \todo default mapping to a potentially random UID is dangerous. This should be fixed */
        $pid=$uid;

    }
    return $pid;
}

/**
 * This function returns the next Person ID in Oracle.
 *
 * @return the corresponding person id (pid) in the Person table in the Oracle database.
 */
function getNextPID(){
    $sql="SELECT max(ID)+1 FROM Person";
    $db = getDAEDatabase();
    return $db->r($sql,array());
}

/**
 * This function returns the Person ID by email
 *
 * @param $email
 * @return PID or -1 if email is not found.
 */
function getPIDByEmail($email){
    /* Unfortunately this code cannot easily be made DBMS independant ... code for OCI only */
    $sql="select h.PERSON_ID from has_More_Email h,Email e where e.ID=h.EMAIL_ID and e.EMAIL=:email";
    $variables[':email'] = $email;
    $db = getDAEDatabase();
    $pid=$db->r($sql,$variables);
    if(empty($pid)){
        return -1;
    }
    return $pid;
}

/**
 * This function handisplaying the Person information.
 * @param $str the multiple-line string to be converted.
 * @param $context where this output string will be embeded in. Currently only support 'js' or 'html'
 * @return the HTML/JavaScript code for displaying multiple-line string. Or the original one if $context is not supported.
 */
function multiple_line($str,$context){
    if(strcmp($context,'js')==0){
        return str_replace("\r","",str_replace("\n","\\n",$str));
    }else if(strcmp($context,'html')==0){
        return str_replace("\r","",str_replace("\n","<br>",$str));
    }
    return $str;
}

class Person{
    function __construct($pid){
        $db = getDAEDatabase();
        settype($pid, 'integer');
        
        $variables[':userpid'] = $pid;
        
        $this->id=$pid;
        $this->name=$db->r('select NAME from Person where ID=:userpid',$variables);
        if(!$this->name){
            $this->name="-";
        }
        if(user_access('view person')){
            $this->aff=$db->r('select AFFILIATION from has_Affiliation h, Affiliation o where h.AFFILIATION_ID=o.ID and h.PERSON_ID=:userpid',$variables);
            if(!$this->aff){
                $this->aff="-";
            }
            $this->email=$db->r('select EMAIL from has_More_Email h, Email o where h.EMAIL_ID=o.ID and h.PERSON_ID=:userpid',$variables);
            if(!$this->email){
                $this->email="-";
            }
            $this->addr=$db->r('select ADDRESS from has_Address h, Address o where h.ADDRESS_ID=o.ID and h.PERSON_ID=:userpid',$variables);
            if(!$this->addr){
                $this->addr="-";
            }
            $this->phone=$db->r('select PHONE_NUMBER from has_Phone_Number h, Phone_Number o where h.PHONE_NUMBER_ID=o.ID and h.PERSON_ID=:userpid',$variables);
            if(!$this->phone){
                $this->phone="-";
            }
            $this->ri=$db->r('select RESEARCH_INTEREST from has_Research_Interest h, Research_Interest o where h.Research_Interest_ID=o.ID and h.PERSON_ID=:userpid',$variables);
            if(!$this->ri){
                $this->ri="-";
            }
        }
    }
    
    /**
     * 
     * This function prepares a string to be stripped of special characters that might cause problems
     * to downstream processes.
     * 
     * @param $str String to format
     * @param $context Context in which the string will be used ('js' or 'html')
     */
    function localStringFormat($str,$context = '') {
        
        $str = str_replace(" ","&nbsp;",$str);
        
        $formatFunction = (strcmp($context,'js')==0 ? 'urlencode' : 'htmlentities' );
        
        return $formatFunction($str);
    }
    
    
    /**
     * This function returns the HTML code of displaying the Person information.
     * @param $context where this output string will be embeded in. Currently only support 'js' or 'html'
     * @return the HTML code of displaying the Person information
     */
    function getHTMLOutputStr($context){
                
        $str="<table>";
        $str.="<tr><td>ID</td><td>".$this->id."</td></tr>";
        $str.="<tr><td>Name</td><td>".$this->localStringFormat($this->name,$context)."</td></tr>";
        if(user_access('view person')){
            $str.="<tr><td>Affiliation</td><td>".$this->localStringFormat($this->aff,$context)."</td></tr>";
            $str.="<tr><td>Email Address</td><td>".$this->localStringFormat($this->email,$context)."</td></tr>";
            $str.="<tr><td>Address</td><td>".$this->localStringFormat($this->addr,$context)."</td></tr>";
            $str.="<tr><td>Phone Number</td><td>".$this->localStringFormat($this->phone,$context)."</td></tr>";
            $str.="<tr><td>Research Interest</td><td>".$this->localStringFormat($this->ri,$context)."</td></tr>";
            $str.="</table>";
        }else{
            $str.="</table><p>You can login to view more information about this person.</p>";
        }

        return $str;
    }

}

