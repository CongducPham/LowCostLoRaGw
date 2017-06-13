<?php
define('LORA_GATEWAY', '/home/pi/lora_gateway'); 

function reboot(){
	return exec("sudo shutdown -r now");
}

function _shutdown(){
	return exec("sudo shutdown -h now");
}

function gw_new_install(){
	return shell_exec("sudo /var/www/html/admin//libs/sh/install_gw.sh");
}

function gw_full_update(){
	return shell_exec("sudo ".LORA_GATEWAY."/scripts/update_gw.sh");
}

function gw_update_file($filename_url){
	return shell_exec("sudo /var/www/html/admin//libs/sh/update_gw_file.sh ".$filename_url);
}

function hostapd_conf($ssid, $wpa_passphrase){
	//echo shell_exec("sudo /var/www/html/admin/sh/wifi.sh ".$ssid." ".$wpa_passphrase." 2>&1");
	return shell_exec("sudo /var/www/html/admin//libs/sh/wifi.sh ".$ssid." ".$wpa_passphrase);
}

//module => Key => value 
//radio_conf => ch => -1
function update_gw_conf($module, $key, $value){
	//echo shell_exec("sudo /var/www/html/admin/sh/gateway_conf.sh ".$module." ".$key." ".$value." 2>&1");
	return shell_exec("sudo /var/www/html/admin/libs/sh/gateway_conf.sh ".$module." ".$key." ".$value);
}

function waziup_key($project, $org, $serv){
	$tree = explode("/", $serv);
	$serv = "";
	for($i=0; $i < sizeof($tree); $i++){
		if($tree[$i] != '')
			$serv .= "\/".$tree[$i];
	}
	$serv = addslashes($serv);
	return shell_exec("sudo /var/www/html/admin//libs/sh/waziup_key.sh ".$project." ".$org." ".$serv);
}

function waziup_conf($key, $value){
	return shell_exec("sudo /var/www/html/admin//libs/sh/waziup_conf.sh ".$key." ".$value);	
}

function thingspeak_key($write_key){
	return shell_exec("sudo /var/www/html/admin//libs/sh/thingspeak_key.sh ".$write_key);
}

function thingspeak_conf($key, $value){
	return shell_exec("sudo /var/www/html/admin//libs/sh/thingspeak_conf.sh ".$key." ".$value);
}

function read_gw_conf_json(){
	$json_src = file_get_contents(LORA_GATEWAY.'/gateway_conf.json');
	return json_decode($json_src, true);
}

function read_clouds_json(){
	$json_src = file_get_contents(LORA_GATEWAY.'/clouds.json');
	return json_decode($json_src, true);
}

function process_gw_conf_json(&$radio_conf, &$gw_conf, &$alert_conf){
	$json_data = read_gw_conf_json();
	if($json_data == null){
		echo "Unable to read gateway_conf.json file </br>";
	}
	else{
		$radio_conf = $json_data['radio_conf'];
		$gw_conf = $json_data['gateway_conf'];
		$alert_conf = $json_data['alert_conf'];
	}
}

function process_clouds_json(&$clouds, &$encrypted_clouds, &$lorawan_encrypted_clouds){
	$json_data = read_clouds_json();
	if($json_data == null){
		echo "Unable to read clouds.json file </br>";
	}
	else{
		$clouds = $json_data['clouds'];
		$encrypted_clouds = $json_data['encrypted_clouds'];
		$lorawan_encrypted_clouds = $json_data['lorawan_encrypted_clouds'];
	}
}

# Enabled field : true or false
function cloud_status($clouds_arr, $cloud_script){
	$i = 0; $find = false;        							
    while($i < count($clouds_arr) && ! $find){
    	if($clouds_arr[$i]['script'] == $cloud_script){
			if($clouds_arr[$i]['enabled']){
				echo "true"; //echo "Enable";   
			}
			else {
				echo "false"; //echo "Disable";  
			}
		}
		$i++;
    }       							
}




?>
