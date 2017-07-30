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

/*************************
 * Move images from uploads to bak
 *************************/
if(isset($_GET['mv']) && $_GET['mv']=="true"){ 
	
	$output1 = shell_exec("sudo cp -r /var/www/html/images/uploads/* /var/www/html/images/bak");

	$output2 = shell_exec("sudo rm -rf /var/www/html/images/uploads/*");
	
	//echo "<pre>$output1</pre>";
	//echo "<pre>$output2</pre>";
	
	if ($output1 == 0 && $output2 == 0){
		echo '<p><center><font color="green">Images moved from uplooads to back directory.</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Issue to move images from uplooads to back directory.</font></center></p>';
	}
	
}



?>
