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

/*********************************
 * Checking Internet connection
 ********************************/
if (isset($_GET['internet']) && $_GET['internet'] == "true"){ 
	if(is_connected())
		echo '<p><center><font color="green">Internet connection successful</font></center></p>';
	else
		echo '<p><center><font color="red">Internet connection failure</font></center></p>';
}

/*********************************
 * Low level status
 ********************************/
if (isset($_GET['low_level_status']) && $_GET['low_level_status'] == "true"){ 	
	$output = low_level_gw_status();
	
	if($output ==""){
		echo '<p><center><font color="red">Desynchronization => '.date('Y-m-d H:i:s').'</font></center></p>';
	}
	else{
		$date = explode(".", $output );
		echo '<p><center><font color="green">Last low-level gateway status ON: '.$date[0].'</font></center></p>';
	}
}

/*********************************
 * Logout
 ********************************/
if (isset($_GET['logout']) && $_GET['logout'] == "true"){ 
	
	// Initialize the session.
	session_start();

	// Unset all of the session variables.
	$_SESSION = array();

	// If it's desired to kill the session, also delete the session cookie.
	// Note: This will destroy the session, and not just the session data!
	if (ini_get("session.use_cookies")) {
    	$params = session_get_cookie_params();
    	setcookie(session_name(), '', time() - 42000,
        	$params["path"], $params["domain"],
        	$params["secure"], $params["httponly"]
    	);
	}

	// Finally, destroy the session.
	session_destroy();

	// redirection
	header('Location: ../');
}

/*************************
 * Gateway: install
 *************************/
if (isset($_GET['gw_new_install']) && $_GET['gw_new_install'] == "true"){ 
	
	// Check internet connection
	if(! is_connected()){
		echo '<p><center><font color="red">No internet connection</font></center></p>';
	}
	else{
		$output = gw_new_install();
		
		if($output == 0){
			echo '<p><center><font color="green">New install of gateway done</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to install a new version of gateway</font></center></p>';	
		}
	}
}

/*************************
 * Gateway: full update
 *************************/
if (isset($_GET['gw_full_update']) && $_GET['gw_full_update'] == "true"){ 
	
	// Check internet connection
	if(! is_connected()){
		echo '<p><center><font color="red">No internet connection</font></center></p>';
	}
	else{	
		$output = gw_full_update(); 
		if($output == 0){
			echo '<p><center><font color="green">Gateway updated</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to update gateway</font></center></p>';	
		}
	}	
}

/*************************
 * Gateway: basic config
 *************************/
if (isset($_GET['gw_basic_conf']) && $_GET['gw_basic_conf'] == "true"){ 
	
	$output = gw_basic_conf();
	if($output == 0){
		echo '<p><center><font color="green">Basic configuration done</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to run basic configuration</font></center></p>';	
	}
}

/*************************
 * Gateway: file update
 *************************/
if (isset($_POST['file_name_url'])){
	 
	 if(empty($_POST['file_name_url'])){
	 	echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	 }
	 else{
		$filename_url = htmlspecialchars($_POST['file_name_url']);
		
		if (filter_var($filename_url, FILTER_VALIDATE_URL) === FALSE) {
    		echo '<p><center><font color="red">Please enter a valid URL</font></center></p>';
		}
		else{
			$output = gw_update_file($filename_url);
	
			if($output == 0){
				echo '<p><center><font color="green">File has been downloaded and installed</font></center></p>';
			}
			else{
				echo '<p><center><font color="red">Failed to install file</font></center></p>';	
			}
		}
	}
}

/*************************
 * Update web admin interface
 *************************/
if (isset($_GET['update_web_admin']) && $_GET['update_web_admin'] == "true"){ 
	
	$output = update_web_admin_interface();
	if($output == 0){
		echo '<p><center><font color="green">Web admin interface updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update web admin interface</font></center></p>';	
	}
}

/*************************
 * Copy post-processing.log
 *************************/
if (isset($_GET['copy_log_file']) && $_GET['copy_log_file'] == "true"){ 
	
	$output = copy_log_file();
	if($output == 0){
		echo '<p><center><font color="green">post-processing.log has been copied and 500 lines have also been extracted. Links are now consistent</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to copy post-processing.log file</font></center></p>';	
	}
}

/*************************
 * Setting mode
 *************************/
if (isset($_GET["mode"]) && (! empty($_GET["mode"]))) {
    
    $mode = htmlspecialchars($_GET["mode"]);
	$output = update_gw_conf("radio_conf","mode", $mode);
	
	if($output == 0){
		echo '<p><center><font color="green">LoRa mode updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update LoRa mode</font></center></p>';	
	}
}

/*************************
 * Setting frequency
 *************************/
if (isset($_GET["band"]) && isset($_GET["freq"])) {
    
	$band = htmlspecialchars($_GET["band"]);
    $freq = htmlspecialchars($_GET["freq"]);
	if (empty($freq) || empty($band)) {
		echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	}
	else{
		$output = update_gw_conf("radio_conf","freq", $freq);
	
		if($output == 0){
			echo '<p><center><font color="green">Frequency updated</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to update frequency</font></center></p>';	
		}
	}
}

/*************************
 * Setting PABOOST
 *************************/
if (isset($_GET["paboost"]) && (! empty($_GET["paboost"]))) {
    
    $paboost = htmlspecialchars($_GET["paboost"]);
	$output = update_paboost($paboost);
	
	if($output == 0){
		echo '<p><center><font color="green">PABOOST option updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update PABOOST option</font></center></p>';	
	}
}

/*************************
 * Setting gateway ID
 *************************/
if (isset($_GET["gateway_ID"])) {
    
    $gw_id = htmlspecialchars($_GET["gateway_ID"]);
	if(empty($gw_id)){
		echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	}
	else{
		$output = update_gw_conf("gateway_conf","gateway_ID", $gw_id);
	
		if($output == 0){
			echo '<p><center><font color="green">Gateway ID updated</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to update gateway ID</font></center></p>';	
		}
	}
}

###################### LAST UPDATE ########################

/*************************
 * Setting raw
 *************************/


if (isset($_GET["raw"])) {
    
    $raw = htmlspecialchars($_GET["raw"]);
	$output = update_gw_conf("gateway_conf","raw", $raw);
	
		if($output == 0){
			echo '<p><center><font color="green">Raw setting updated</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to update raw setting</font></center></p>';	
		}
}



/*************************
 * Setting wappkey
 *************************/


if (isset($_GET["wappkey"])) {
    
    $wappkey = htmlspecialchars($_GET["wappkey"]);
	$output = update_gw_conf("gateway_conf","wappkey", $wappkey);
	
		if($output == 0){
			echo '<p><center><font color="green">Wappkey setting updated</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to update wappkey setting</font></center></p>';	
		}
}

/*************************
 * Setting localization
 *************************/
 
 if (isset($_GET["ref_latitude"])&&isset($_GET["ref_longitude"])) {
    
    $ref_latitude = htmlspecialchars($_GET["ref_latitude"]);
	$ref_latitude = '\"'.$ref_latitude.'\"';
	$output = update_gw_conf("gateway_conf","ref_latitude", $ref_latitude);
	if($output == 0){
		echo '<p><center><font color="green">Latitude setting updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update latitude setting</font></center></p>';	
	}
	$ref_longitude = htmlspecialchars($_GET["ref_longitude"]);
	$ref_longitude = '\"'.$ref_longitude.'\"';
	$output = update_gw_conf("gateway_conf","ref_longitude", $ref_longitude);
	
	if($output == 0){
		echo '<p><center><font color="green">Longitude setting updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update longitude setting</font></center></p>';	
	}
}
 
################### 
#Status of other clouds
#	-cloudNoInternet
# 	-cloudGpsFile
# 	-cloudMQTT
# 	-cloudNodeRed
 ###################
if (
	isset($_GET["cloud_status"]) && (! empty($_GET["cloud_status"])) &&
	isset($_GET["cloud_name"]) && (! empty($_GET["cloud_name"]))
){
	$cloud_status = htmlspecialchars($_GET["cloud_status"]);
	$cloud_name = htmlspecialchars($_GET["cloud_name"]);
	
	$output =clouds_conf("enabled", $cloud_status,$cloud_name);
	
	if($output == 0){
		echo '<p><center><font color="green">Cloud status updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update cloud status</font></center></p>';	
	}
} 
 
################### 
#Keys of other clouds
#	-cloudNoInternet
# 	-cloudGpsFile
# 	-cloudMQTT
# 	-cloudNodeRed
 ###################
if(
	isset($_GET["clouds_key_name"]) && (! empty($_GET["clouds_key_name"])) &&
	isset($_GET["clouds_key"]) && (! empty($_GET["clouds_key"])) &&
	isset($_GET["clouds_key_value"]) && (! empty($_GET["clouds_key_value"]))
){
	$cloud_key  = htmlspecialchars($_GET["clouds_key"]);
	$cloud_key_name = htmlspecialchars($_GET["clouds_key_name"]);
	$cloud_key_value =  htmlspecialchars($_GET["clouds_key_value"]);


	$output =clouds_conf($cloud_key ,$cloud_key_value,$cloud_key_name );

	if($output == 0){
		switch($cloud_key_name){
			case "gpsfile_key":
				echo '<p><center><font color="green">Cloud GPS file updated</font></center></p>';
				break;
			case "nointernet_key":
				echo '<p><center><font color="green">Cloud No Internet updated</font></center></p>';
				break;
			case "mqtt_key":
				echo '<p><center><font color="green">Cloud MQTT updated</font></center></p>';
				break;
			case "nodered_key":
				echo '<p><center><font color="green">Cloud Node-Red updated</font></center></p>';
				break;
			default :
				echo '<p><center><font color="green">Cloud updated</font></center></p>';
		}	
	}
	else{
		switch($cloud_key_name){
			case "gpsfile_key":
				echo '<p><center><font color="red">Failed to update cloud GPS file</font></center></p>';	
				break;
			case "nointernet_key":
				echo '<p><center><font color="red">Failed to update cloud No Internet</font></center></p>';	
				break;
			case "mqtt_key":
				echo '<p><center><font color="red">Failed to update cloud MQTT</font></center></p>';	
				break;
			case "nodered_key":
				echo '<p><center><font color="red">Failed to update cloud Node-Red</font></center></p>';	
				break;
			default :
				echo '<p><center><font color="red">Failed to update cloud</font></center></p>';
		}
	}
}


if(
	isset($_GET["clouds_list"]) && (! empty($_GET["clouds_list"])) &&
	isset($_GET["clouds_key"]) && (! empty($_GET["clouds_key"])) &&
	isset($_GET["clouds_key_value"]) && (! empty($_GET["clouds_key_value"]))
){
	$cloud_key  = htmlspecialchars($_GET["clouds_key"]);
	$cloud_key_name = htmlspecialchars($_GET["clouds_list"]);
	$cloud_key_value =  json_decode($_GET["clouds_key_value"]);
	
	if((count($cloud_key_value) == 1) && ($cloud_key_value[0] =="")){
		$str = '[]';
	}else{
		$str = '[';
			for($i = 0; $i < sizeof($cloud_key_value); $i++ ){	
				if($i == (sizeof($cloud_key_value)-1))		
					$str .= '\"'.$cloud_key_value[$i].'\"';
				else
					$str .= '\"'.$cloud_key_value[$i].'\",';
			}
		$str .= ']';
	}
	$output =clouds_conf($cloud_key ,$str,$cloud_key_name );
	if($output == 0){
		echo '<p><center><font color="green">List updated</font></center></p>';
	}else{
		echo '<p><center><font color="red">Failed to update list</font></center></p>';
	}
}

###########################################################



/*************************
 * Setting AES
 *************************/
if (isset($_GET["aes"]) && (! empty($_GET["aes"]))) {
    
    $aes = htmlspecialchars($_GET["aes"]);
	$output = update_gw_conf("gateway_conf","aes", $aes);
	
	if($output == 0){
		echo '<p><center><font color="green">AES option updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update AES option</font></center></p>';	
	}
}

/*************************
 * Setting downlink
 *************************/
if (isset($_GET["downlink"]) && (! empty($_GET["downlink"]))) {
    
    $downlink = htmlspecialchars($_GET["downlink"]);
    
    if ($downlink == '') {
    	$downlink=0;
    }
    	
	$output = update_gw_conf("gateway_conf","downlink", $downlink);
	
	if($output == 0){
		echo '<p><center><font color="green">downlink timer updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update downlink timer</font></center></p>';	
	}
}

/*************************
 * Setting ThingSpeak status
 *************************/
if (isset($_GET["thingspeak_status"]) && (! empty($_GET["thingspeak_status"]))) {
    
    $thingspeak_status = htmlspecialchars($_GET["thingspeak_status"]);
	$output = thingspeak_conf("enabled", $thingspeak_status);
	
	if($output == 0){
		echo '<p><center><font color="green">ThingSpeak status updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update ThingSpeak status</font></center></p>';	
	}
}







/*************************
 * Setting Waziup status
 *************************/
if (isset($_GET["waziup_status"]) && (! empty($_GET["waziup_status"]))) {
    
    $waziup_status = htmlspecialchars($_GET["waziup_status"]);
	$output = waziup_conf("enabled", $waziup_status);
	
	if($output == 0){
		echo '<p><center><font color="green">Waziup cloud status updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update Waziup cloud status</font></center></p>';	
	}
}


/*************************
 * Setting alert mail
 *************************/
if (isset($_GET["use_mail"]) && (! empty($_GET["use_mail"]))) {
    
    $use_mail = htmlspecialchars($_GET["use_mail"]);
	$output = update_gw_conf("alert_conf","use_mail", $use_mail);
	
	if($output == 0){
		echo '<p><center><font color="green">Alert mail info updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update alert mail info</font></center></p>';	
	}
}

/*************************
 * Setting mail account
 *************************/
if (isset($_GET["mail_from"]) && (! empty($_GET["mail_from"]))) {
    
    $mail_from = htmlspecialchars($_GET["mail_from"]);
     if (empty($mail_from)) {
	echo '<p><center><font color="red">Please fill all fields</font></center></p>';
    }
    else{
	$output = update_gw_conf("alert_conf","mail_from", $mail_from);
	
	if($output == 0){
		echo '<p><center><font color="green">Mail account updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update mail account</font></center></p>';	
	}
    }
}

/*************************
 * Setting mail password
 *************************/
if (isset($_GET["mail_passwd"]) ) {
    
    $mail_passwd = htmlspecialchars($_GET["mail_passwd"]);

	if(empty($mail_passwd)){
		echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	}
	else{
		$output = update_gw_conf("alert_conf","mail_passwd", $mail_passwd);
		if($output == 0){
			echo '<p><center><font color="green">Mail password updated</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to update mail password</font></center></p>';	
		}
	}
	
}

/*************************
 * Setting mail server
 *************************/
if (isset($_GET["mail_server"])) {
	
    $mail_server = htmlspecialchars($_GET["mail_server"]);
	
	if(empty($mail_server)){
		echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	}
	else{
		$output = update_gw_conf("alert_conf","mail_server", $mail_server);
		if($output == 0){
			echo '<p><center><font color="green">Mail server updated</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to update mail server</font></center></p>';	
		}
	}	
}

/*************************
 * Setting contact mail
 *************************/
if (isset($_GET["contact_mail"]) && (! empty($_GET["contact_mail"]))) {
    $contacts = json_decode($_GET["contact_mail"]);
   	//var_dump($contacts);
  
	$str="";
	for($i = 0; $i < sizeof($contacts); $i++ ){	
		if($i == sizeof($contacts)-1)	
			$str .= $contacts[$i];
		else
			$str .= $contacts[$i].",";
	}
		
	$output = update_contact_mail($str);
	
	if($output == 0){
		echo '<p><center><font color="green">Mail contacts updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update mail contacts</font></center></p>';	
	}
}

/*************************
 * Setting alert sms
 *************************/
if (isset($_GET["use_sms"]) && (! empty($_GET["use_sms"]))) {
    
    $use_sms = htmlspecialchars($_GET["use_sms"]);
	$output = update_gw_conf("alert_conf","use_sms", $use_sms);
	
	if($output == 0){
		echo '<p><center><font color="green">Alert SMS updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update alert SMS</font></center></p>';	
	}
}

/*************************
 * Setting sms pin
 *************************/
if (isset($_GET["sms_pin"]) && (! empty($_GET["sms_pin"]))) {
    
    $sms_pin = htmlspecialchars($_GET["sms_pin"]);
	$output = update_gw_conf("alert_conf","pin", $sms_pin);
	
	if($output == 0){
		echo '<p><center><font color="green">Pin code updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update pin code</font></center></p>';	
	}
}

/*************************
 * Setting contact sms
 *************************/
if (isset($_GET["contact_sms"]) && (! empty($_GET["contact_sms"]))) {
    $contacts = json_decode($_GET["contact_sms"]);
   	
   	//valid list number: [\"+33299002233\",\"+23299338822\",\"+47922337788"]
	$str = '[';
	for($i = 0; $i < sizeof($contacts); $i++ ){	
		if($i == (sizeof($contacts)-1))		
			$str .= '\"'.$contacts[$i].'\"';
		else
			$str .= '\"'.$contacts[$i].'\",';
	}
	$str .= ']';		
	echo $str; 
	
	$output = update_contact_sms($str);
	
	if($output == 0){
		echo '<p><center><font color="green">SMS contacts updated</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to update SMS contacts</font></center></p>';	
	}
}


/*************************
 * Setting access point
 *************************/
if(isset($_POST['ssid']) && isset($_POST['wpa_phrasepass'])){
	
	$ssid = htmlspecialchars($_POST['ssid']);
    $wpa_pass = htmlspecialchars($_POST['wpa_phrasepass']);
    
	if(empty($ssid) || empty($wpa_pass)){
		echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	}
	else{
    
		$output = hostapd_conf($ssid, $wpa_pass);
	
		if($output == 0){
			echo '<p><center><font color="green">WiFi access point configuration updated. You MUST reboot for changes to take effect</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to update WiFi access point configuration</font></center></p>';
		}
	}
}

/*************************
 * Setting wifi client
 *************************/
if(isset($_POST['wificlient_ssid']) && isset($_POST['wificlient_wpa_phrasepass'])){
	
	$wificlient_ssid = htmlspecialchars($_POST['wificlient_ssid']);
    $wificlient_wpa_pass = htmlspecialchars($_POST['wificlient_wpa_phrasepass']);
    
	if(empty($wificlient_ssid) || empty($wificlient_wpa_pass)){
		echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	}
	else{ 
    
		$output = wificlient_conf($wificlient_ssid, $wificlient_wpa_pass);
	
		if($output == 0){
			echo '<p><center><font color="green">WiFi client network configuration updated. You MUST reboot for changes to take effect</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to update WiFi client configuration</font></center></p>';
		}
	}
}

/*************************
 * Setting ap mode
 *************************/

if (isset($_GET['apmode']) && $_GET['apmode'] == "true"){ 
    
	$output = apmode_conf();

	if($output == 0){
		echo '<p><center><font color="green">Gateway is configured for access point mode. You MUST reboot for changes to take effect</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to configure gateway in access point mode</font></center></p>';
	}
}	

/*************************
 * Setting ap mode now
 *************************/

if (isset($_GET['apmodenow']) && $_GET['apmodenow'] == "true"){ 
    
	$output = apmodenow_conf();

	if($output == 0){
		echo '<p><center><font color="green">Gateway is now in access point mode. Connect to gateway at 192.168.200.1</font></center></p>';
	}
	else{
		echo '<p><center><font color="red">Failed to set gateway in access point mode</font></center></p>';
	}
}	

/******************************
 * Setting ThingSpeak write key
 *****************************/
if(isset($_GET['write_key'])){
	
	$write_key = htmlspecialchars($_GET['write_key']);
    
	if(empty($write_key)){
		echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	}
	else{ 
	
		$output = thingspeak_key($write_key);
	
		if($output == 0){
			echo '<p><center><font color="green">Write key saved</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to save write key</font></center></p>';
		}
	}
}

/******************************
 * Setting Waziup project name
 ******************************/
if(isset($_GET['project_name'])){
	
	$project = htmlspecialchars($_GET['project_name']);
    
	if(empty($project)){
		echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	}
	else{ 
    
		$output = waziup_key("project_name", $project);
	
		if($output == 0){
			echo '<p><center><font color="green">Project name saved</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to save project name</font></center></p>';
		}
	}
}

/***********************************
 * Setting Waziup organization name
 ***********************************/
if(isset($_GET['organization_name'])){
	
    $org = htmlspecialchars($_GET['organization_name']);
    
	if(empty($org)){
		echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	}
	else{
    
		$output = waziup_key("organization_name", $org);
	
		if($output == 0){
			echo '<p><center><font color="green">Organization name saved</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to save organization name</font></center></p>';
		}
	}
}

/******************************
 * Setting Waziup service tree
 ******************************/
if(isset($_GET['service_tree'])){
	
    $serv = htmlspecialchars($_GET['service_tree']);
    echo $serv;
	if(empty($serv)){
		echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	}
	else{
    
		$output = waziup_key("service_tree", $serv);
	
		if($output == 0){
			echo '<p><center><font color="green">Service tree saved</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to save service tree</font></center></p>';
		}
	}
}

/******************************
 * Setting Waziup Orion token
 ******************************/
if(isset($_GET['auth_token'])){
	
    $auth_token = htmlspecialchars($_GET['auth_token']);
    echo $auth_token;
	if(empty($auth_token)){
		echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	}
	else{
    
		$output = waziup_key("auth_token", $auth_token);
	
		if($output == 0){
			echo '<p><center><font color="green">Auth token saved</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to save Auth token</font></center></p>';
		}
	}
}

/*************************
 * Setting downlink request
 *************************/
if(isset($_POST['destination']) && isset($_POST['message'])){
	
	$dst = htmlspecialchars($_POST['destination']);
    $msg = htmlspecialchars($_POST['message']);
    
	if(empty($dst) || empty($msg)){
		echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	}
	else{ 
		// not empty : $dst and $msg
		$str = '"';
		$str .= $msg;
		$str .= '"';
		$output = send_downlink($dst, $str);
	
		if($output == 0){
			echo '<p><center><font color="green">Downlink request saved</font></center></p>';
		}
		else{
			echo '<p><center><font color="red">Failed to save downlink request</font></center></p>';
		}
	}
}

/*************************
 * Setting profile
 *************************/
if(isset($_POST['current_username'], $_POST['new_username'], $_POST['current_pwd'], $_POST['new_pwd'])){
	
	$c_usr = htmlspecialchars($_POST['current_username']);
	$n_usr = htmlspecialchars($_POST['new_username']);
    $c_pwd = htmlspecialchars($_POST['current_pwd']);
    $n_pwd = htmlspecialchars($_POST['new_pwd']);
    
	session_start();

	if(empty($c_usr) || empty($n_usr) || empty($c_pwd) || empty($n_pwd)){
		echo '<p><center><font color="red">Please fill all fields</font></center></p>';
	}
	else{ 
		/*
		echo 'Current username='.$c_usr.'</br>';
		echo 'Current pwd='.$c_pwd.'</br>';
		echo 'Current pwd md5='.md5($c_pwd).'</br>';
		echo '$_SESSION["username"]='.$_SESSION['username'].'</br>';
		echo '$_SESSION["password"]='.$_SESSION['password'].'</br>';
		*/
		if(! check_login($c_usr, md5($c_pwd), $_SESSION['username'], $_SESSION['password'])){
			echo '<p><center><font color="red">Please enter correct connection settings</font></center></p>';
		}
		else{
			$output = set_profile($n_usr, md5($n_pwd));
			if($output == 0){
				echo '<p><center><font color="green">Profile updated</font></center></p>';
				echo '<p><center><font color="green">Please logout then login again using new connection settings</font></center></p>';
			}
			else{
				echo '<p><center><font color="red">Failed to set profile</font></center></p>';
			}
		}
	}
}

?>
