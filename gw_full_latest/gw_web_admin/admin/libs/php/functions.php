<?php
define('LORA_GATEWAY', '/home/pi/lora_gateway'); 

function reboot(){
	return exec("sudo shutdown -r now");
}

function _shutdown(){
	return exec("sudo shutdown -h now");
}

function is_connected(){
	return fopen("http://www.google.com:80/","r");
}

function low_level_gw_status(){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh low_level_gw_status");
}

function gw_new_install(){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh install_gw");
}

function gw_full_update(){
	return shell_exec("sudo ".LORA_GATEWAY."/scripts/update_gw.sh");
}

function gw_basic_conf(){
	return shell_exec("sudo ".LORA_GATEWAY."/scripts/basic_config_gw.sh");
}

function gw_update_file($filename_url){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh update_gw_file ".$filename_url);
}

function update_web_admin_interface(){
	return shell_exec("sudo ".LORA_GATEWAY."/gw_web_admin/install.sh");
}

function copy_log_file(){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh copy_log_file");
}

function hostapd_conf($ssid, $wpa_passphrase){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh wifi ".$ssid." ".$wpa_passphrase);
}

function wificlient_conf($wificlient_ssid, $wificlient_wpa_passphrase){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh wificlient ".$wificlient_ssid." ".$wificlient_wpa_passphrase);
}

function apmode_conf(){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh apmode");
}

function apmodenow_conf(){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh apmodenow");
}

//module => Key => value 
//radio_conf => ch => -1
function update_gw_conf($module, $key, $value){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh gateway_conf ".$module." ".$key." ".$value);
}

function update_paboost($value) {
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh paboost ".$value);
}

function update_contact_sms($contacts){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh contact_sms ".$contacts);
}

function update_contact_mail($contacts){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh contact_mail ".$contacts);
}

function waziup_key($key_name, $key_value){
	if($key_name == "service_tree"){
		$tree = explode("-", $key_value);
		$key_value = "";
		for($i=0; $i < sizeof($tree); $i++){
			if($tree[$i] != '')
				$key_value .= "\-".$tree[$i];
		}
		//$key_value = addslashes($key_value);
	}
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh waziup_key ".$key_name." ".$key_value);
}

function waziup_conf($key, $value){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh waziup_conf ".$key." ".$value);	
}

function thingspeak_key($write_key){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh thingspeak_key ".$write_key);
}

function thingspeak_conf($key, $value){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh thingspeak_conf ".$key." ".$value);
}

/*For other clouds***************/
function clouds_conf($key, $value,$conf){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh ".$conf." ".$key." ".$value);
}
/********************************/
function read_gw_conf_json(){
	$json_src = file_get_contents(LORA_GATEWAY.'/gateway_conf.json');
	return json_decode($json_src, true);
}

function read_clouds_json(){
	$json_src = file_get_contents(LORA_GATEWAY.'/clouds.json');
	return json_decode($json_src, true);
}

function read_database_json(){
	$json_src = file_get_contents('/etc/gw_web_admin/database.json');
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

function process_key_clouds(){
	$key_waziup = "/home/pi/lora_gateway/key_WAZIUP.py";
	if(is_file($key_waziup)){	
		return json_decode(exec('python /var/www/html/admin/libs/python/key_clouds.py'), true);
	}
	else{
		return json_decode(exec('python /var/www/html/admin/libs/python/key_thingspeak.py'), true);
	}
}

# Enabled field : true or false
function cloud_status($clouds_arr, $cloud_script){
	$i = 0; $find = false;        							
    while($i < count($clouds_arr) && ! $find){
    	if($clouds_arr[$i]['script'] == $cloud_script){
			if($clouds_arr[$i]['enabled']){
				echo "true"; 
			}
			else {
				echo "false"; 
			}
		}
		$i++;
    }       							
}

function send_downlink($dst, $msg){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh downlink_request ".$dst." ".$msg);
}

function display_array($arr){
	for($i = 0; $i < sizeof($arr); $i++ ){	
		if($i == sizeof($arr)-1)		
			echo $arr[$i];
		else
			echo $arr[$i].", ";
	}
}

function check_login($db_log, $db_pwd, $form_log, $form_pwd){
	return $db_log==$form_log && $db_pwd==$form_pwd;
}

function set_profile($new_username, $new_password){
	return shell_exec("sudo /var/www/html/admin/libs/sh/web_shell_script.sh set_profile ".$new_username." ".$new_password);
}

?>
