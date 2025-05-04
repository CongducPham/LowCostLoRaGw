<?php
include_once 'libs/php/functions.php';

$main_dir = 'bak/node_'; // main directory
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

/**********************************
 * Move images from uploads to bak
 *********************************/
if(isset($_GET['mv']) && $_GET['mv']=="true"){ 
	
	$output = shell_exec("sudo /var/www/html/images/libs/sh/move_img.sh");
	
	if ($output == 0){
		echo '<p><center><font color="green">Images moved from uplooads to back directory.</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Issue to move images from uplooads to back directory.</font></center></p>';
	}
	
}

?>