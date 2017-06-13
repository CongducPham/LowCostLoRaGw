<?php
include_once '../libs/php/functions.php';

/*************************
 * Reboot gateway
 *************************/
// if $_GET['reboot'] exists and is "true"
if (isset($_GET['reboot']) && $_GET['reboot'] == "true"){
	reboot();
}

/*************************
 * Shutdomn gateway
 *************************/
if (isset($_GET['shutdown']) && $_GET['shutdown'] == "true"){ 
	_shutdown();
}

/*************************
 * Gateway: install
 *************************/
if (isset($_GET['gw_new_install']) && $_GET['gw_new_install'] == "true"){ 
	
	$output = gw_new_install(); 
	if($output == 0){
		//echo "Success";
		echo '<p><center><font color="green">Update of gateway done.</font></center></p>';
	}
	else{
		//echo "Failure";
		echo '<p><center><font color="green">Issue to install a new version of gateway</font></center></p>';	
	}
}

/*************************
 * Gateway: full update
 *************************/
if (isset($_GET['gw_full_update']) && $_GET['gw_full_update'] == "true"){ 
	
	$output = gw_full_update();
	if($output == 0){
		//echo "Success";
		echo '<p><center><font color="green">Gateway updated.</font></center></p>';
	}
	else{
		//echo "Failure";
		echo '<p><center><font color="red">Issue to update gateway</font></center></p>';	
	}
}

/*************************
 * Gateway: file update
 *************************/
if (isset($_POST['file_name_url'])){
	 
	 if(empty($_POST['file_name_url'])){
	 	echo '<p><center><font color="red">Please fill all fields.</font></center></p>';
	 }
	 else{
	 	//$filename_url = htmlspecialchars($_GET['file_name_url']);
		$filename_url = $_POST['file_name_url'];
		//echo $filename_url ;
		$output = gw_update_file($filename_url);
	
		if($output == 0){
			//echo "Success";
			echo '<p><center><font color="green">Gateway updated.</font></center></p>';
		}
		else{
			//echo "Failure";
			echo '<p><center><font color="red">Issue to update gateway</font></center></p>';	
		}
	 }
}

/*************************
 * Setting mode
 *************************/
if (isset($_GET["mode"]) && (! empty($_GET["mode"]))) {
    
    $mode = htmlspecialchars($_GET["mode"]);
	$output = update_gw_conf("radio_conf","mode", $mode);
	
	if($output == 0){
		//echo "Success";
		echo '<p><center><font color="green">Mode updated.</font></center></p>';
	}
	else{
		//echo "Failure";
		echo '<p><center><font color="red">Issue to update mode.</font></center></p>';	
	}
}

/*************************
 * Setting frequency
 *************************/
if (isset($_GET["freq"]) && (! empty($_GET["freq"]))) {
    
    $freq = htmlspecialchars($_GET["freq"]);
	$output = update_gw_conf("radio_conf","freq", $freq);
	
	if($output == 0){
		//echo "Success";
		echo '<p><center><font color="green">Frequency updated.</font></center></p>';
	}
	else{
		//echo "Failure";
		echo '<p><center><font color="red">Issue to update frequency.</font></center></p>';	
	}
}


/*************************
 * Setting gateway ID
 *************************/
if (isset($_GET["gateway_ID"]) && (! empty($_GET["gateway_ID"]))) {
    
    $gw_id = htmlspecialchars($_GET["gateway_ID"]);
	$output = update_gw_conf("gateway_conf","gateway_ID", $gw_id);
	
	if($output == 0){
		//echo "Success";
		echo '<p><center><font color="green">Gateway ID updated</font></center></p>';
	}
	else{
		//echo "Failure";
		echo '<p><center><font color="red">Issue to update gateway ID.</font></center></p>';	
	}
}

/*************************
 * Setting AES
 *************************/
if (isset($_GET["aes"]) && (! empty($_GET["aes"]))) {
    
    $aes = htmlspecialchars($_GET["aes"]);
	$output = update_gw_conf("gateway_conf","aes", $aes);
	
	if($output == 0){
		//echo "Success";
		echo '<p><center><font color="green">AES updated.</font></center></p>';
	}
	else{
		//echo "Failure";
		echo '<p><center><font color="red">Issue to update AES.</font></center></p>';	
	}
}

/*************************
 * Setting ThingSpeak status
 *************************/
if (isset($_GET["thingspeak_status"]) && (! empty($_GET["thingspeak_status"]))) {
    
    $thingspeak_status = htmlspecialchars($_GET["thingspeak_status"]);
	$output = thingspeak_conf("enabled", $thingspeak_status);
	
	if($output == 0){
		//echo "Success";
		echo '<p><center><font color="green">ThingSpeak status updated.</font></center></p>';
		//echo $thingspeak_status;
	}
	else{
		//echo "Failure";
		echo '<p><center><font color="red">Issue to update ThingSpeak status.</font></center></p>';	
	}
}

/*************************
 * Setting Waziup status
 *************************/
if (isset($_GET["waziup_status"]) && (! empty($_GET["waziup_status"]))) {
    
    $waziup_status = htmlspecialchars($_GET["waziup_status"]);
	$output = waziup_conf("enabled", $waziup_status);
	
	if($output == 0){
		//echo "Success";
		echo '<p><center><font color="green">Waziup Orion status updated.</font></center></p>';
		//echo $waziup_status;
	}
	else{
		//echo "Failure";
		echo '<p><center><font color="red">Issue to update Waziup Orion status.</font></center></p>';	
	}
}


/*************************
 * Setting alert mail
 *************************/
if (isset($_GET["use_mail"]) && (! empty($_GET["use_mail"]))) {
    
    $use_mail = htmlspecialchars($_GET["use_mail"]);
	$output = update_gw_conf("alert_conf","use_mail", $use_mail);
	
	if($output == 0){
		//echo "Success";
		echo '<p><center><font color="green">Alert mail updated.</font></center></p>';
	}
	else{
		//echo "Failure";
		echo '<p><center><font color="red">Issue to update alert mail.</font></center></p>';	
	}
}


/*************************
 * Setting alert sms
 *************************/
if (isset($_GET["use_sms"]) && (! empty($_GET["use_sms"]))) {
    
    $use_sms = htmlspecialchars($_GET["use_sms"]);
	$output = update_gw_conf("alert_conf","use_sms", $use_sms);
	
	if($output == 0){
		//echo "Success";
		echo '<p><center><font color="green">Alert sms updated.</font></center></p>';
	}
	else{
		//echo "Failure";
		echo '<p><center><font color="red">Issue to update alert sms.</font></center></p>';	
	}
}


/*************************
 * Setting access point
 *************************/
if(isset($_POST['ssid']) && isset($_POST['wpa_phrasepass'])){
	
	$ssid = htmlspecialchars($_POST['ssid']);
    $wpa_pass = htmlspecialchars($_POST['wpa_phrasepass']);
    
	if((empty($ssid) || empty($wpa_pass)) || (empty($ssid) && empty($wpa_pass))){
		echo '<p><center><font color="red">Please fill all fields.</font></center></p>';
	}
	else{ // not empty: isset($_POST['ssid']) && isset($_POST['wpa_phrasepass'])
		//Gerer le contr^le de saisie ici
		
		//S'il n'y a aucune erreur faire ceci
    
		$output = hostapd_conf($ssid, $wpa_pass);
	
		if($output == 0){
			//echo "Success";
			echo '<p><center><font color="green">Access point updated.</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Issue to update the access point.</font></center></p>';
		}
	}
}

/*************************
 * Setting ThingSpeak
 *************************/
if(isset($_POST['write_key'])){
	
	$write_key = htmlspecialchars($_POST['write_key']);
    
	if(empty($write_key)){
		echo '<p><center><font color="red">Please fill all fields.</font></center></p>';
	}
	else{ // not empty: isset($_POST['ssid']) && isset($_POST['wpa_phrasepass'])
		//Gerer le contr^le de saisie ici
		
		//S'il n'y a aucune erreur faire ceci
		$output = thingspeak_key($write_key);
	
		if($output == 0){
			//echo "Success";
			echo '<p><center><font color="green">ThingSpeak configuration saved.</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Issue to save ThingSpeak configuration.</font></center></p>';
		}
	}
}

/*************************
 * Setting Waziup
 *************************/
if(isset($_POST['project_name']) && isset($_POST['organization_name']) && isset($_POST['service_tree'])){
	
	$project = htmlspecialchars($_POST['project_name']);
    $org = htmlspecialchars($_POST['organization_name']);
    $serv = htmlspecialchars($_POST['service_tree']);
    
	if((empty($project) || empty($org) || empty($serv)) || (empty($project) && empty($org) && empty($serv))){
		echo '<p><center><font color="red">Please fill all fields.</font></center></p>';
	}
	else{ // not empty: isset($_POST['ssid']) && isset($_POST['wpa_phrasepass'])
		//Gerer le contr^le de saisie ici
		
		//S'il n'y a aucune erreur faire ceci
    
		$output = waziup_key($project, $org, $serv);
	
		if($output == 0){
			//echo "Success";
			echo '<p><center><font color="green">Waziup configuration saved.</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Issue to save Waziup configuration.</font></center></p>';
		}
	}
}

?>
