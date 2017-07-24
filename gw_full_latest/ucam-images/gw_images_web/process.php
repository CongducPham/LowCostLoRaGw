<?php
include_once 'libs/php/functions.php';

$main_dir = 'uploads/node_'; // main directory
$extensions = array('bmp'); // picture extensions array: add or remove extensions

/*************************
 * Video node selection
 *************************/
if(isset($_GET['node']) && isset($_GET['time_interval'])){ 
	
	$node = htmlspecialchars($_GET['node']);
    $time_interval = htmlspecialchars($_GET['time_interval']);
    //echo $node;
    //echo $time_interval;
	if(empty($node) || empty($time_interval)){
		echo '<p><center><font color="red">Please fill all fields.</font></center></p>';
	}
	else{
		echo  display_captures_per_node($main_dir, $extensions, $node, $time_interval);
	}
}



?>
