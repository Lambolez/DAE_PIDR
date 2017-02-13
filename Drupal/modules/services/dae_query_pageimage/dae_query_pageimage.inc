<?php

require_once("./".drupal_get_path('module','daedatabase')."/daedatabase_db.inc");
require_once("./".drupal_get_path('module','dae_data')."/dae_data_query_client.inc");

function dae_query_pageimage($lim,$pitList,$dsList,$petList,$pepList,$pevList,$fpit,$fds,$fpet,$fpep) {
	
	$sql=genSQL($lim,$pitList,$dsList,$petList,$pepList,$pevList,$fpit,$fds,$fpet,$fpep);
	$all=array();
	$db = getDAEDatabase();
	$result=$db->q($sql,$variable);
	while($row = $db->fetch_array($result)){
		$id=$row['ID'];
		array_push($all,$id);
	}
	return $all;
}

