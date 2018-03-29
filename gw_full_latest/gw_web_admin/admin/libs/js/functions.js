// shorter syntax for document.ready
$(function () {
//####################### LAST UPDATE #######################

var maxAddr = 255;

//#######################
//# Setting Cloud No Internet status
//#######################
	$('#td_edit_cloudNoInternet_status').hide();
	$('#cloudNoInternet_status_msg').hide();
	$('#td_cloudNoInternet_status_submit').hide();
	
	$('#btn_edit_cloudNoInternet_status').click(function() {	
		$('#td_edit_cloudNoInternet_status').show();
		$('#td_cloudNoInternet_status_submit').show();
	});
	
	$('#btn_cloudNoInternet_status_submit').click(function(){
	    	var cloud_status = $( "#div_cloudNoInternet_status_options input:checked" ).val();
	    	var cloud_name = "cloudnointernet_conf";
	    	$.get("process.php", {cloud_status: cloud_status,cloud_name: cloud_name}, function(data){	
				$('#cloud_msg').html(data);
		    		//erase message after 5 seconds
		   		 setTimeout(function() {
	  				$('#cloud_msg').html("");
		   		 },5000);
		});
	   	
		if(cloud_status != ''){
			$('#cloudNoInternet_status_value').html(cloud_status);
			$('#td_edit_cloudNoInternet_status').hide();
	    		$('#td_cloudNoInternet_status_submit').hide();
		}	
	});
	
//#######################
//# Setting Cloud Gps File status
//#######################


	$('#td_edit_cloudGpsFile_status').hide();
	$('#td_cloudGpsFile_status_submit').hide();
	
	$('#btn_edit_cloudGpsFile_status').click(function(){
		$('#td_edit_cloudGpsFile_status').show();
		$('#td_cloudGpsFile_status_submit').show();
	});
	
	$('#btn_cloudGpsFile_status_submit').click(function(){
	    	var cloud_status = $( "#div_cloudGpsFile_status_options input:checked" ).val();
	    	var cloud_name = "cloudgpsfile_conf";
	    	$.get("process.php", {cloud_status: cloud_status,cloud_name: cloud_name}, function(data){	
				$('#cloud_msg').html(data);
		    		//erase message after 5 seconds
		   		 setTimeout(function() {
	  				$('#cloud_msg').html("");
		   		 },5000);
		});
	    	
		if(cloud_status != '' ){
			$('#cloudGpsFile_status_value').html(cloud_status);
			$('#td_edit_cloudGpsFile_status').hide();
	    		$('#td_cloudGpsFile_status_submit').hide();
		}	
	});
	
//#######################
//# Setting Cloud Gps File project name
//#######################
	$('#td_edit_cloudGpsFile_project_name').hide();
	$('#td_cloudGpsFile_project_name_submit').hide();

	$('#btn_edit_cloudGpsFile_project_name').click(function(){
		$('#td_edit_cloudGpsFile_project_name').show();
		$('#td_cloudGpsFile_project_name_submit').show();
	});


 	$('#btn_cloudGpsFile_project_name_submit').click(function(){
		var cloud_key_value = $('#cloudGpsFile_project_name_input').val();
		var cloud_key_name = "gpsfile_key";
		var cloud_key = "project_name";
	    	$.get("process.php", {clouds_key_name:cloud_key_name, clouds_key:cloud_key, clouds_key_value:cloud_key_value}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#cloudGpsFile_project_name_value').html(cloud_key_value );
			$('#td_edit_cloudGpsFile_project_name').hide();
			$('#td_cloudGpsFile_project_name_submit').hide();
		}	
	});
//#######################
//# Setting Cloud Gps File organization name
//#######################
	$('#td_edit_cloudGpsFile_organization_name').hide();
	$('#td_cloudGpsFile_organization_name_submit').hide();

	$('#btn_edit_cloudGpsFile_organization_name').click(function(){
		$('#td_edit_cloudGpsFile_organization_name').show();
		$('#td_cloudGpsFile_organization_name_submit').show();
	});

 	$('#btn_cloudGpsFile_organization_name_submit').click(function(){
		var cloud_key_value = $('#cloudGpsFile_organization_name_input').val();
		var cloud_key_name = "gpsfile_key";
		var cloud_key = "organization_name";
	    	$.get("process.php", {clouds_key_name:cloud_key_name, clouds_key:cloud_key, clouds_key_value:cloud_key_value}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#cloudGpsFile_project_name_value').html(cloud_key_value );
			$('#td_edit_cloudGpsFile_organization_name').hide();
			$('#td_cloudGpsFile_organization_name_submit').hide();
		}	
	});

//#######################
//# Setting Cloud Gps File sensor name
//#######################
	$('#td_edit_cloudGpsFile_sensor_name').hide();
	$('#td_cloudGpsFile_sensor_name_submit').hide();

	$('#btn_edit_cloudGpsFile_sensor_name').click(function(){
		$('#td_edit_cloudGpsFile_sensor_name').show();
		$('#td_cloudGpsFile_sensor_name_submit').show();
	});
 	$('#btn_cloudGpsFile_sensor_name_submit').click(function(){
		var cloud_key_value = $('#cloudGpsFile_sensor_name_input').val();
		var cloud_key_name = "gpsfile_key";
		var cloud_key = "sensor_name";
	    	$.get("process.php", {clouds_key_name:cloud_key_name, clouds_key:cloud_key, clouds_key_value:cloud_key_value}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#cloudGpsFile_sensor_name_value').html(cloud_key_value );
			$('#td_edit_cloudGpsFile_sensor_name').hide();
			$('#td_cloudGpsFile_sensor_name_submit').hide();
		}	
	});

//#######################
//# Setting Cloud Gps File source list
//#######################
	$('#td_edit_cloudGpsFile_source_list').hide();
	$('#td_cloudGpsFile_source_list_submit').hide();

	$('#btn_edit_cloudGpsFile_source_list').click(function(){
		$('#td_edit_cloudGpsFile_source_list').show();
		$('#td_cloudGpsFile_source_list_submit').show();
	});
 	$('#btn_cloudGpsFile_source_list_submit').click(function(){
		var cloud_key_value = $('#cloudGpsFile_source_list_input').val();
		var cloud_key_name = "gpsfile_key";
		var cloud_key = "source_list";

		cloud_key_value = String(cloud_key_value);
		var buffer = cloud_key_value.split(",");
		for(x in buffer){
			buffer[x] = buffer[x].split(' ').join("");
			var n = parseInt(buffer[x]);
			if(n === NaN || n < 2 || n > maxAddr){
				console.log(n);
				$('#cloud_msg').html("Error on range of sensor, must be beetween 2 and "+maxAddr);
				return -1;
			}					
		}
		var keyJSON = JSON.stringify(buffer);
		cloud_key_value = keyJSON;		

	    	$.get("process.php", {clouds_list : cloud_key_name, clouds_key : cloud_key, clouds_key_value : keyJSON}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#cloudGpsFile_source_list_value').html(cloud_key_value);
			$('#td_edit_cloudGpsFile_source_list').hide();
			$('#td_cloudGpsFile_source_list_submit').hide();
		}
	});

//#######################
//# Setting Cloud Gps File active interval
//#######################
	$('#td_edit_cloudGpsFile_active_interval').hide();
	$('#td_cloudGpsFile_active_interval_submit').hide();

	$('#btn_edit_cloudGpsFile_active_interval').click(function(){
		$('#td_edit_cloudGpsFile_active_interval').show();
		$('#td_cloudGpsFile_active_interval_submit').show();
	});
	
 	$('#btn_cloudGpsFile_active_interval_submit').click(function(){
		var cloud_key_value = $('#cloudGpsFile_active_interval_input').val();
		var cloud_key_name = "gpsfile_key";
		var cloud_key = "active_interval_minutes";

		var test = parseInt(cloud_key_value); 
		if (test == NaN){
			$('#cloud_msg').html("It is not a number !");
			return -1; 
		}
	    	$.get("process.php", {clouds_key_name:cloud_key_name, clouds_key:cloud_key, clouds_key_value:clouds_key_value}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#cloudGpsFile_active_interval_value').html(cloud_key_value );
			$('#td_edit_cloudGpsFile_active_interval').hide();
			$('#td_cloudGpsFile_active_interval_submit').hide();
		}	
	});
//#######################
//# Setting Cloud Gps File sms
//#######################
	$('#td_edit_cloudGpsFile_sms').hide();
	$('#td_cloudGpsFile_sms_submit').hide();

	$('#btn_edit_cloudGpsFile_sms').click(function(){
		$('#td_edit_cloudGpsFile_sms').show();
		$('#td_cloudGpsFile_sms_submit').show();
	});
	
 	$('#btn_cloudGpsFile_sms_submit').click(function(){
		var cloud_key_value = $('#div_cloudGpsFile_sms input:checked').val();
		var cloud_key_name = "gpsfile_key";
		var cloud_key = "sms";
		
	    	$.get("process.php", {clouds_key_name:cloud_key_name, clouds_key:cloud_key, clouds_key_value:cloud_key_value}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			if(cloud_key_value === "true"){
				$('#cloudGpsFile_sms_value').html("Enable");
			}else{
				$('#cloudGpsFile_sms_value').html("Disable");
			}			
			$('#td_edit_cloudGpsFile_sms').hide();
			$('#td_cloudGpsFile_sms_submit').hide();
		}	
	});
//#######################
//# Setting Cloud Gps File pin
//#######################
	$('#td_edit_cloudGpsFile_pin').hide();
	$('#td_cloudGpsFile_pin_submit').hide();

	$('#btn_edit_cloudGpsFile_pin').click(function(){
		$('#td_edit_cloudGpsFile_pin').show();
		$('#td_cloudGpsFile_pin_submit').show();
	});
	
 	$('#btn_cloudGpsFile_pin_submit').click(function(){
		var cloud_key_value = $('#cloudGpsFile_pin_input').val();
		var cloud_key_name = "gpsfile_key";
		var cloud_key = "sms";
		
	    	$.get("process.php", {clouds_key_name:cloud_key_name, clouds_key:cloud_key, clouds_key_value:cloud_key_value}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#cloudGpsFile_pin_value').html(cloud_key_value );
			$('#td_edit_cloudGpsFile_pin').hide();
			$('#td_cloudGpsFile_pin_submit').hide();
		}	
	});

//#######################
//# Setting Cloud Gps Contacts
//#######################

	$('#td_edit_cloudGpsFile_contacts').hide();
	$('#td_cloudGpsFile_contacts_submit').hide();

	$('#btn_edit_cloudGpsFile_contacts').click(function(){
		$('#td_edit_cloudGpsFile_contacts').show();
		$('#td_cloudGpsFile_contacts_submit').show();
	});
 	$('#btn_cloudGpsFile_contacts_submit').click(function(){
		var cloud_key_value = $('#cloudGpsFile_contacts_input').val();
		var cloud_key_name = "gpsfile_key";
		var cloud_key = "contacts";
		

		cloud_key_value = String(cloud_key_value);
		var buffer = cloud_key_value.split(",");
		var valid = true;
		for(x in buffer){
			buffer[x] = buffer[x].split(' ').join("");
			if (! /^[\+]?[(]?[0-9]{3}[)]?[-\s\.]?[0-9]{3}[-\s\.]?[0-9]{4,9}$/.test(buffer[x]))
			{
				$('#cloud_msg').html("Error : "+buffer[x]+", must be a phone number");	
				return -1;
			}
			console.log(buffer[x]);
		}
		var keyJSON = JSON.stringify(buffer);
		cloud_key_value = keyJSON;

	    	$.get("process.php", {clouds_list : cloud_key_name, clouds_key : cloud_key, clouds_key_value : cloud_key_value}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#cloudGpsFile_contacts_value').html(cloud_key_value );
			$('#td_edit_cloudGpsFile_contacts').hide();
			$('#td_cloudGpsFile_contacts_submit').hide();
		}	
	});

//#######################
//# Setting Cloud MQTT status
//#######################


	$('#td_edit_cloudMQTT_status').hide();
	$('#td_cloudMQTT_status_submit').hide();

	$('#btn_edit_cloudMQTT_status').click(function(){
		$('#td_edit_cloudMQTT_status').show();
		$('#td_cloudMQTT_status_submit').show();
	});
	
	$('#btn_cloudMQTT_status_submit').click(function(){
	    	var cloud_status = $( "#div_cloudMQTT_status_options input:checked" ).val();
	    	var cloud_name = "cloudmqtt_conf";
	    	$.get("process.php", {cloud_status: cloud_status,cloud_name: cloud_name}, function(data){	
				$('#cloud_msg').html(data);
		    		//erase message after 5 seconds
		   		 setTimeout(function() {
	  				$('#cloud_msg').html("");
		   		 },5000);
		});
	    	
		if(cloud_status != ''){
			$('#cloudMQTT_status_value').html(cloud_status);
			$('#td_edit_cloudMQTT_status').hide();
	    		$('#td_cloudMQTT_status_submit').hide();
		}
	});
	
//#######################
//# Setting Cloud Node Red status
//#######################


	$('#td_edit_cloudNodeRed_status').hide();
	$('#td_cloudNodeRed_status_submit').hide();
	
	$('#btn_edit_cloudNodeRed_status').click(function(){
		$('#td_edit_cloudNodeRed_status').show();
		$('#td_cloudNodeRed_status_submit').show();
	});
	
	$('#btn_cloudNodeRed_status_submit').click(function(){
	    	var cloud_status = $( "#div_cloudNodeRed_status_options input:checked" ).val();
	    	var cloud_name = "cloudnodered_conf";
	    	$.get("process.php", {cloud_status: cloud_status,cloud_name: cloud_name}, function(data){	
				$('#cloud_msg').html(data);
		    		//erase message after 5 seconds
		   		 setTimeout(function() {
	  				$('#cloud_msg').html("");
		   		 },5000);
		});
	    	
		if(cloud_status != ''){
			$('#cloudNodeRed_status_value').html(cloud_status);
			$('#td_edit_cloudNodeRed_status').hide();
	    		$('#td_cloudNodeRed_status_submit').hide();
		}
	
	});
	
//#######################
//# Gateway settings localization 
//#######################	
	$('#td_edit_gw_position').hide();
	$('#td_format_position_dd').hide();
	$('#td_format_position_dms').hide();
	$('#td_submit_position').hide();
	$('#div_info_format').hide();

	$('#btn_edit_gw_position').click(function(){
		$('#td_edit_gw_position').show();
		$('#td_submit_position').show();
	});
	$('#format_dd').click(function(){
		//$('#div_info_format').html("");
		$('#div_info_format').hide();
		$('#td_format_position_dd').show();
		$('#td_format_position_dms').hide();
	});
	$('#format_dms').click(function(){

		var info = "Negative numbers are not allowed</br>Separate integral part from decimal with a dot"+
					", i-e : 42.36</p></br>Minimum of five numbers after the dot for more accuracy. ";
		$('#div_info_format').html(info);
		$('#div_info_format').show();
		$('#td_format_position_dms').show();
		$('#td_format_position_dd').hide();
	});
	
	$('#btn_submit_position').click(function(){
		var ref_latitude = 0;
		var ref_longitude = 0;
		if($('#div_update_format_position input:checked').val() === "dd"){
			ref_latitude = parseFloat($("#latitude_dd_input").val());
			ref_longitude = parseFloat($("#longitude_dd_input").val());
			if(ref_longitude === NaN || ref_latitude ===NaN){
				$('#gw_config_msg').html("invalid coordinates");
			}
		}
		if($('#div_update_format_position input:checked').val() === "dms"){
			
			var latitude_degree = parseFloat($('#latitude_degree_input').val()); 
			var latitude_minutes = parseFloat($('#latitude_minute_input').val()); 
			var latitude_seconds = parseFloat($('#latitude_second_input').val()); 

			var longitude_degree = parseFloat($('#longitude_degree_input').val()); 
			var longitude_minutes = parseFloat($('#longitude_minute_input').val()); 
			var longitude_seconds = parseFloat($('#longitude_second_input').val()); 
		
			if($('#latitude_group input:checked').val() ==="S")
			{
				latitude_degree = -latitude_degree ;
			}
			if($('#latitude_group input:checked').val() ==="W")
			{
				longitude_degree= -longitude_degree;
			}
		
			if(latitude_degree >= 0){
			  	ref_latitude = latitude_seconds/3600 + latitude_minutes/60 + latitude_degree ;
			}else{
				ref_latitude = - ( latitude_seconds/3600) - ( latitude_minutes/60) + latitude_degree;
			}
			
			if(longitude_degree < 0){
			  	ref_longitude = longitude_seconds/3600 + longitude_minutes/60 +longitude_degree;
			}else{
				ref_longitude = - (longitude_seconds/3600) - (longitude_minutes/60) + longitude_degree; 
			}	
		}
		//alert("ref_latitude : "+ref_latitude+"ref_longitude : "+ref_longitude );
		$.get("process.php", {ref_latitude: ref_latitude.toString(),ref_longitude: ref_longitude.toString()}, function(data){	
				$("#gw_config_msg").html(data);
		    			//erase message after 5 seconds
		   			setTimeout(function() {
	  					$('#gw_config_msg').html("");
		     			},5000);
		});
		$('#latitude_gw_conf').html(ref_latitude);
		$('#longitude_gw_conf').html(ref_longitude);
		$('#latitude_value').html("Latitude :"+ref_latitude);
		$('#longitude_value').html("Longitude :"+ref_longitude);
		$('#td_edit_gw_position').hide();
		$('#td_format_position_dd').hide();
		$('#td_format_position_dms').hide();
		$('#td_submit_position').hide();
	});
	
//#######################
//# Gateway display localization 
//#######################

	$('#latitude_gw_conf').hide();
	$('#longitude_gw_conf').hide();
/*
	$('#display_format_dms').click(function(){
		var val_latitude = parseFloat($('#latitude_gw_conf').html());
		var val_longitude = parseFloat($('#longitude_gw_conf').html());

		var latitude_dms = convertDdToDms(val_latitude);
		var longitude_dms = convertDdToDms(val_longitude);
	
		$('#latitude_value').html("Latitude : "+latitude_dms);
		$('#longitude_value').html("Longitude : "+longitude_dms);
	
	});
*/

	function convertDdToDms(val){
		var val_abs = Math.abs(val)
		var degree = Math.trunc(val_abs);
		var minutes = Math.trunc((val_abs - degree) * 60);
		var seconds = (val_abs - degree - (minutes/60)) * 3600;
		var display ="";
		if(val < 0){
			display = "-"+degree+"° "+minutes+"\' "+seconds+"\"";
		}else{
			display = degree+"° "+minutes+"\' "+seconds+"\"";
		}
	
		return display;
	}
/*
	$('#display_format_dd').click(function(){
		$('#latitude_value').html("Latitude : "+$('#latitude_gw_conf').html());
		$('#longitude_value').html("Longitude : "+$('#longitude_gw_conf').html());
	});
*/		
//#######################
//# Setting raw
//#######################	
 	$('#td_edit_raw').hide();
	$('#td_raw_submit').hide();
	
	$('#btn_edit_raw').click(function(){
		$('#td_edit_raw').show();
		$('#td_raw_submit').show();
	});
	
	$('#btn_raw_submit').click(function(){
		raw = $( "#raw_group input:checked" ).val();
		alert(raw);
		$.get("process.php", {raw: raw}, function(data){	
			$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
        });
    	
		if(raw != ''){
    		$('#raw_value').html(raw);
        	$('#td_edit_raw').hide();
    		$('#td_raw_submit').hide();
    	}
	});

//#######################
//# Setting wappkey
//#######################	
 	$('#td_edit_wappkey').hide();
	$('#td_wappkey_submit').hide();
	
	$('#btn_edit_wappkey').click(function(){
		$('#td_edit_wappkey').show();
		$('#td_wappkey_submit').show();
	});
	
	$('#btn_wappkey_submit').click(function(){
		wappkey = $( "#wappkey_group input:checked" ).val();
		alert(wappkey);
		$.get("process.php", {wappkey: wappkey}, function(data){	
			$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
        });
    	
		if(wappkey != ''){
    		$('#wappkey_value').html(wappkey);
        	$('#td_edit_wappkey').hide();
    		$('#td_wappkey_submit').hide();
    	}
	});





//######################
//# thingspeak_source_list
//######################

$('#td_edit_thingspeak_source_list').hide();
$('#td_thingspeak_source_list_submit').hide();

$('#btn_edit_thingspeak_source_list').click(function(){
	$('#td_edit_thingspeak_source_list').show();
	$('#td_thingspeak_source_list_submit').show();
});

$('#btn_thingspeak_source_list_submit').click(function(){
		var cloud_key_value = $('#thingspeak_source_list_input').val();
		var cloud_key_name = "thingspeak_key";
		var cloud_key = "source_list";

		
		cloud_key_value = String(cloud_key_value);
		var buffer = cloud_key_value.split(",");
		for(x in buffer){
			buffer[x] = buffer[x].split(' ').join("");
			var n = parseInt(buffer[x]);
			if(n === NaN || n < 2 || n > maxAddr){
				console.log(n);
				$('#cloud_msg').html("Error on range of sensor, must be beetween 2 and "+maxAddr);
				return -1;
			}					
		}
		var keyJSON = JSON.stringify(buffer);
		cloud_key_value = keyJSON;		

	    	$.get("process.php", {clouds_list : cloud_key_name, clouds_key : cloud_key, clouds_key_value : keyJSON}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#td_thingspeak_source_list_value').html(cloud_key_value);
			$('#td_edit_thingspeak_source_list').hide();
			$('#td_thingspeak_source_list_submit').hide();
		}
});

//######################
//# Waziup_source_list
//######################

$('#td_edit_waziup_source_list').hide();
$('#td_waziup_source_list_submit').hide();

$('#btn_edit_waziup_source_list').click(function(){
	$('#td_edit_waziup_source_list').show();
	$('#td_waziup_source_list_submit').show();
});

$('#btn_waziup_source_list_submit').click(function(){
		var cloud_key_value = $('#waziup_source_list_input').val();
		var cloud_key_name = "waziup_key";
		var cloud_key = "source_list";

		
		cloud_key_value = String(cloud_key_value);
		var buffer = cloud_key_value.split(",");
		for(x in buffer){
			buffer[x] = buffer[x].split(' ').join("");
			var n = parseInt(buffer[x]);
			if(n === NaN || n < 2 || n > maxAddr){
				console.log(n);
				$('#cloud_msg').html("Error on range of sensor, must be beetween 2 and "+maxAddr);
				return -1;
			}					
		}
		var keyJSON = JSON.stringify(buffer);
		cloud_key_value = keyJSON;		

	    	$.get("process.php", {clouds_list : cloud_key_name, clouds_key : cloud_key, clouds_key_value : keyJSON}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#td_waziup_source_list_value').html(cloud_key_value);
			$('#td_edit_waziup_source_list').hide();
			$('#td_waziup_source_list_submit').hide();
		}
});

//#######################
//#	CloudMQTT MQTT server
//#######################
	$('#td_edit_cloudMQTT_MQTT_server').hide();
	$('#td_cloudMQTT_MQTT_server_submit').hide();

	$('#btn_edit_cloudMQTT_MQTT_server').click(function(){
		$('#td_edit_cloudMQTT_MQTT_server').show();
		$('#td_cloudMQTT_MQTT_server_submit').show();
	});


 	$('#btn_cloudMQTT_MQTT_server_submit').click(function(){
		var cloud_key_value = $('#cloudMQTT_MQTT_server_input').val();
		var cloud_key_name = "mqtt_key";
		var cloud_key = "MQTT_server";
	    	$.get("process.php", {clouds_key_name:cloud_key_name, clouds_key:cloud_key, clouds_key_value:cloud_key_value}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#td_cloudMQTT_MQTT_server_value').html(cloud_key_value );
			$('#td_edit_cloudMQTT_MQTT_server').hide();
			$('#td_cloudMQTT_MQTT_server_submit').hide();
		}	
	});


//#######################
//#	CloudMQTT project name
//#######################
	$('#td_edit_cloudMQTT_project_name').hide();
	$('#td_cloudMQTT_project_name_submit').hide();

	$('#btn_edit_cloudMQTT_project_name').click(function(){
		$('#td_edit_cloudMQTT_project_name').show();
		$('#td_cloudMQTT_project_name_submit').show();
	});


 	$('#btn_cloudMQTT_project_name_submit').click(function(){
		var cloud_key_value = $('#cloudMQTT_project_name_input').val();
		var cloud_key_name = "mqtt_key";
		var cloud_key = "project_name";
	    	$.get("process.php", {clouds_key_name:cloud_key_name, clouds_key:cloud_key, clouds_key_value:cloud_key_value}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#td_cloudMQTT_project_name_value').html(cloud_key_value );
			$('#td_edit_cloudMQTT_project_name').hide();
			$('#td_cloudMQTT_project_name_submit').hide();
		}	
	});

//#######################
//#	CloudMQTT oraganization name
//#######################
	$('#td_edit_cloudMQTT_organization_name').hide();
	$('#td_cloudMQTT_organization_name_submit').hide();

	$('#btn_edit_cloudMQTT_organization_name').click(function(){
		$('#td_edit_cloudMQTT_organization_name').show();
		$('#td_cloudMQTT_organization_name_submit').show();
	});


 	$('#btn_cloudMQTT_organization_name_submit').click(function(){
		var cloud_key_value = $('#cloudMQTT_organization_name_input').val();
		var cloud_key_name = "mqtt_key";
		var cloud_key = "organization_name";
	    	$.get("process.php", {clouds_key_name:cloud_key_name, clouds_key:cloud_key, clouds_key_value:cloud_key_value}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#td_cloudMQTT_organization_name_value').html(cloud_key_value );
			$('#td_edit_cloudMQTT_organization_name').hide();
			$('#td_cloudMQTT_organization_name_submit').hide();
		}	
	});

//#######################
//#	CloudMQTT sensor  name
//#######################
	$('#td_edit_cloudMQTT_sensor_name').hide();
	$('#td_cloudMQTT_sensor_name_submit').hide();

	$('#btn_edit_cloudMQTT_sensor_name').click(function(){
		$('#td_edit_cloudMQTT_sensor_name').show();
		$('#td_cloudMQTT_sensor_name_submit').show();
	});


 	$('#btn_cloudMQTT_sensor_name_submit').click(function(){
		var cloud_key_value = $('#cloudMQTT_sensor_name_input').val();
		var cloud_key_name = "mqtt_key";
		var cloud_key = "sensor_name";
	    	$.get("process.php", {clouds_key_name:cloud_key_name, clouds_key:cloud_key, clouds_key_value:cloud_key_value}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#td_cloudMQTT_sensor_name_value').html(cloud_key_value );
			$('#td_edit_cloudMQTT_sensor_name').hide();
			$('#td_cloudMQTT_sensor_name_submit').hide();
		}	
	});

//#######################
//#	CloudMQTT source list
//#######################
$('#td_edit_cloudMQTT_source_list').hide();
$('#td_cloudMQTT_source_list_submit').hide();

$('#btn_edit_cloudMQTT_source_list').click(function(){
	$('#td_edit_cloudMQTT_source_list').show();
	$('#td_cloudMQTT_source_list_submit').show();
});

$('#btn_cloudMQTT_source_list_submit').click(function(){
		var cloud_key_value = $('#cloudMQTT_source_list_input').val();
		var cloud_key_name = "mqtt_key";
		var cloud_key = "source_list";

		
		cloud_key_value = String(cloud_key_value);
		var buffer = cloud_key_value.split(",");
		for(x in buffer){
			buffer[x] = buffer[x].split(' ').join("");
			var n = parseInt(buffer[x]);
			if(n === NaN || n < 2 || n > maxAddr){
				console.log(n);
				$('#cloud_msg').html("Error on range of sensor, must be beetween 2 and "+maxAddr);
				return -1;
			}					
		}
		var keyJSON = JSON.stringify(buffer);
		cloud_key_value = keyJSON;		

	    	$.get("process.php", {clouds_list : cloud_key_name, clouds_key : cloud_key, clouds_key_value : keyJSON}, function(data){	
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
		});
	    	
		if(cloud_key_value  != ''){
			$('#td_cloudMQTT_source_list_value').html(cloud_key_value);
			$('#td_edit_cloudMQTT_source_list').hide();
			$('#td_cloudMQTT_source_list_submit').hide();
		}
});
//##########################################################	
	
	
//######################
//# Reboot
//######################
   /*
	$('#btn_reboot').click(function() {
        $.post($(this).attr('href'), function (res) { 
            // handle the response (*) by storing the message into the DIV#message
            //$('#msg').html(res);
        });
    });
   */
    
    $("#btn_reboot").click(function(e) {
        e.preventDefault();
        $("#confirm_reboot").modal("show");
    });
    
    $("#cancel_reboot").click(function() {
        $("#confirm_reboot").modal("hide");
    });
    
    $("#ok_reboot").click(function() {
        //$("#confirm_reboot").modal("hide");

        $.get("process.php", {reboot: 'true'}, function(data){
            //alert(data);
        });
    
    }); 
  
//######################
//# Shutdown
//######################
   /*	
	$('#btn_shutdown').click(function() {
        $.post($(this).attr('href'), function (res) { 
            // handle the response (*) by storing the message into the DIV#message
            //$('#msg').html(res);
        });
    });
   */  
    
    $("#btn_shutdown").click(function(e) {
        e.preventDefault();
        $("#confirm_shutdown").modal("show");
    });
    
    $("#cancel_shutdown").click(function() {
        $("#confirm_shutdown").modal("hide");
        //alert("Cancel");
    });
    
    $("#ok_shutdown").click(function() {
        //$("#confirm_shutdown").modal("hide");
    
        $.get("process.php", {shutdown: 'true'}, function(data){
        	//alert(data);
        });
    
    });
    
//#################################
//# Checking Internet connection
//#################################
	
	$('#btn_internet').click(function() {
        	$.post($(this).attr('href'), function (data) { 
           	//display message
           	$('#header_msg').html(data);
          		 //erase message after 5 seconds
			setTimeout(function() {
  				$('#header_msg').html("");
			},5000);
        	});
   	 });
 
 //#################################
//# Low level status
//#################################
	$('#btn_low_level_status').click(function() {
        $.post($(this).attr('href'), function (data) { 
           	$('#header_msg').html(data);
          		 //erase message after 9 seconds
			setTimeout(function() {
  				$('#header_msg').html("");
			},9000);
       	});
   	 });

//#################################
//# Logout
//#################################
	
	$('#logout').click(function() {
		//alert("cool");
        $.post($(this).attr('href'), function (data) { 
           //display message
	   alert(data);
           //$('#header_msg').html(data);
           //erase message after 5 seconds
	   //		setTimeout(function() {
  	   //			$('#header_msg').html("");
	   //		},5000);
        });
    });
    
//###########################
//# Gateway: install
//###########################
	$('#btn_gw_new_install').click(function() {
      // Confirm
		var response = confirm('Do you want to perform a new install with latest version of gateway?');
		if (response == true){ // OK clicked
			msg = '<p><center><font color="green">New installation in progress, wait for finish notification.</font></center></p>';
			$('#gw_update_msg').html(msg);

			$.post($(this).attr('href'), function (res) { 
            	// handle the response (*) by storing the message into the DIV#message
	   			//$('#gw_update_msg').html("");
	    		$('#gw_update_msg').html(res);
	    		//erase message after 5 seconds
	    		setTimeout(function() {
  					$('#gw_update_msg').html("");
  					window.location.reload();  					
	     		},5000);
	 		});
		}
		else{ 
			// Cancel clicked
		}
    });

//######################
//# Gateway: full update
//######################
	$('#btn_gw_full_update').click(function() {
        //confirm        
		var response = confirm('Do you want to perform a full update to latest version of gateway?');
		if (response == true){ // OK clicked
			msg = '<p><center><font color="green">Full update in progress, wait for finish notification.</font></center></p>';
			$('#gw_update_msg').html(msg);

			$.post($(this).attr('href'), function (res) { 
            	// handle the response (*) by storing the message into the DIV#message
             	//$('#gw_update_msg').html("");
	    		$('#gw_update_msg').html(res);
	    		//erase message after 5 seconds
	   		 	setTimeout(function() {
  					$('#gw_update_msg').html("");
  					window.location.reload();  					
	    		},5000);
        	});
		}
		else{ 
			// Cancel clicked
		}
    });

//#######################
//# Gateway: basic config
//#######################
	$('#btn_gw_basic_conf').click(function() {
        msg = '<p><center><font color="green">Basic gateway configuration in progress, wait for finish notification.</font></center></p>';
        $('#gw_update_msg').html(msg);	
        $.post($(this).attr('href'), function (res) { 
            // handle the response (*) by storing the message into the DIV#message
            //$('#gw_basic_conf_msg').html(res);
	     $('#gw_update_msg').html(res);
	    //erase message after 5 seconds
	    setTimeout(function() {
  		$('#gw_update_msg').html("");
  		window.location.reload();  		
	    },5000);
        });
    });
      
//#######################
//# Gateway: update file
//#######################
	$('#btn_gw_update_form_reset').click(function() {	
    	$('#gw_update_form')[0].reset();
    	$("#gw_update_form_msg").html("");
    });	

	$('#gw_update_form').submit(function(event){
        // Stop form from submitting normally
        event.preventDefault();
        
        /* Serialize the submitted form control values to be sent to the web server with the request */
        var formValues = $(this).serialize();
        
        // Send the form data using post
        $.post("process.php", formValues, function(data){
            // Display the returned data in browser
            //$("#gw_update_form_msg").html(data);
	    $('#gw_update_msg').html(data);
	    //erase message after 5 seconds
	    setTimeout(function() {
  		$('#gw_update_msg').html("");
	    },5000);
        });
        $('#gw_update_form')[0].reset();    
        
    });
    
//######################
//#  Update web admin interface
//######################
	$('#btn_update_web_admin').click(function() {
        msg = '<p><center><font color="green">Web admin interface update in progress, wait for finish notification.</font></center></p>';
        $('#gw_update_msg').html(msg);	
        $.post($(this).attr('href'), function (res) { 
            // handle the response (*) by storing the message into the DIV#message
            //$('#gw_full_update_msg').html(res);
	     $('#gw_update_msg').html(res);
	    //erase message after 5 seconds
	    setTimeout(function() {
  		$('#gw_update_msg').html("");
  		window.location.reload();  		
	    },5000);
        });
    });

//######################
//#  Copy log file
//######################
	$('#btn_copy_log_file').click(function() {
        $.post($(this).attr('href'), function (res) { 
            // handle the response (*) by storing the message into the DIV#message
            //$('#gw_full_update_msg').html(res);
	     $('#gw_config_msg').html(res);
	    //erase message after 5 seconds
	    setTimeout(function() {
  		$('#gw_config_msg').html("");
	    },5000);
        });
    });

//#######################
//# Setting ThingSpeak
//#######################
	
	$('#btn_thingspeak_form_reset').click(function() {	
    	$('#hostapd_form')[0].reset();
    	$("#thingspeak_form_msg").html("");
    });	

	$("#thingspeak_form").submit(function(event){
        // Stop form from submitting normally
        event.preventDefault();
        
        /* Serialize the submitted form control values to be sent to the web server with the request */
        var formValues = $(this).serialize();
        
        // Send the form data using post
        $.post("process.php", formValues, function(data){
            // Display the returned data in browser
            //$("#thingspeak_form_msg").html(data);
	     $('#cloud_msg').html(data);
	    //erase message after 5 seconds
	    setTimeout(function() {
  		$('#cloud_msg').html("");
	    },5000);
        });
        $('#thingspeak_form')[0].reset();
        
    });

//######################
//# Setting Mode
//######################
    $('#td_edit_mode').hide();
    $('#mode_submit').hide();
    $("#radio_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_mode').click(function() {	
    	$('#td_edit_mode').show();
    	$('#mode_submit').show();
    });
 
    $('#mode_submit').click(function() {	
    	mode = $('#mode_select').val();
    	
    	if(mode == ''){
    		//alert("Please select one mode!");
		setTimeout(function(){
			$('#radio_msg').html("Please select one mode!");
		
}, 3000);
    	}
    	else{ 
    		$.get("process.php", {mode: mode}, function(data){
				//$('#radio_msg').html(data);
				//$('#radio_msg').show();
				$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
        	});
    		
        	$('#mode_value').html(mode);
        	$('#td_edit_mode').hide();
    		$('#mode_submit').hide();
    		//$('#mode_select').html($('#mode_select option:selected').val());
    		//$('#mode_select option:selected').text();
    		//setTimeOut(function(){$('#radio_msg').hide();}, 3000);
    		//$('#radio_msg').show().delay(3000).hide();
    		//$("#radio_msg").hide().delay(3000);
    	}
    });
    
//######################
//# Setting frequency
//######################
	$('#td_edit_freq').hide();
    $('#freq_submit').hide();
    $("#radio_msg").hide();
    
    $('#btn_edit_freq').click(function() {	
    	$('#td_edit_freq').show();
    	$('#freq_submit').show();
    });
    
    $('#band_freq_select').on('change', function() {	
    	band = $('#band_freq_select').val();
    	
    	if(band == ''){
    		//alert("Please select one frequency band!");
    		$('#freq_select').html("");
    	}
    	else if(band == "433MHz"){
    		$('#freq_select').html("<option>433.3</option><option>433.6</option><option>433.9</option><option>434.3</option>");
    	}
    	else if(band == "868MHz"){
    		$('#freq_select').html("<option>863.2</option><option>863.5</option><option>863.8</option>"+
    								"<option>864.1</option><option>864.4</option><option>864.7</option>"+
    								"<option selected>865.2</option><option>865.5</option><option>865.8</option>"+
    								"<option>866.1</option><option>866.4</option><option>866.7</option>"+
    								"<option>867.0</option><option>868.0</option><option>868.1</option>");
    	}
    	else{ // 915 MHz
    		$('#freq_select').html("<option>903.08</option><option>905.24</option><option>907.40</option>"+
    								"<option>909.56</option><option>911.72</option><option selected>913.88</option>"+
    								"<option>915.0</option><option>916.04</option><option>918.20</option><option>920.36</option>"+
    								"<option>922.52</option><option>924.68</option><option>926.84</option>");
    	}
    	
    });
    
    
    $('#freq_submit').click(function() {	
    	band = $('#band_freq_select').val();
	freq = $('#freq_select').val();
    	
	$.get("process.php", {band:band, freq: freq}, function(data){
            	// Display the returned data in browser
            	//$("#mode_test").html(data);
            	//alert(data);
		//$('#radio_msg').html(data);
		//$('#radio_msg').show();
		$("#gw_config_msg").html(data);
            	//erase message after 5 seconds
	   	setTimeout(function() {
  			$('#gw_config_msg').html("");
	     	},5000);
        });
	
	if(freq != '' && band != ''){
		$('#freq_value').html(freq);
	}
        $('#td_edit_freq').hide();
    	$('#freq_submit').hide();
	
	
	/*
	$band = $('#band_freq_select').val();
    	if($band == ''){
    		alert("Please select one frequency band!");
    	}
    	else{
    		$freq = $('#freq_select').val();

    		if($freq == ''){
    			alert("Please select one frequency!");
    		}
    		else{
    			// test de contrôle sur le mode
    				$.get("process.php", {freq: $freq}, function(data){
            		// Display the returned data in browser
            		//$("#mode_test").html(data);
            		//alert(data);
					$('#radio_msg').html(data);
					$('#radio_msg').show();
        		});
    			
    			//$.post($(this).attr('href'), function (res) { 
            		//$('#msg').html(res);
        		//});
        		$('#freq_value').html($freq);
        		$('#td_edit_freq').hide();
    			$('#freq_submit').hide();
    		}
    	}*/
    	
    });

//######################
//# Setting PABOOST
//######################
    $('#td_edit_paboost').hide();
    $('#td_paboost_submit').hide();
    $("#gw_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_paboost').click(function() {	
    	$('#td_edit_paboost').show();
    	$('#td_paboost_submit').show();
    });
 
    $('#btn_paboost_submit').click(function() {	
    	paboost = $( "input:checked" ).val();
        $.get("process.php", {paboost: paboost}, function(data){
            // handle the response (*) by storing the message into the DIV#message
            //$('#gw_full_update_msg').html(res);
	     $('#gw_config_msg').html(data);
	    //erase message after 5 seconds
	    setTimeout(function() {
  		$('#gw_config_msg').html("");
	    },5000);
        });
    	
	if(paboost != '') 
        	$('#paboost_value').html(paboost);
        $('#td_edit_paboost').hide();
    	$('#td_paboost_submit').hide();
    });
       										   
    
//######################
//# Setting Gateway ID
//######################
    $('#td_edit_gw_id').hide();
    $('#td_gw_id_submit').hide();
    $("#gw_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_gw_id').click(function() {	
    	$('#td_edit_gw_id').show();
    	$('#td_gw_id_submit').show();
    });
 
    $('#gw_id_submit').click(function() {	
    	gw_id = $('#gw_id_input').val();
    	
    	$.get("process.php", {gateway_ID: gw_id}, function(data){	
			//$('#gw_msg').html(data);
			//$('#gw_msg').show();
			$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
        });
    	
	if(gw_id != '')	
        	$('#gw_id_value').html(gw_id);
        $('#gw_id_input').val("");
        $('#td_edit_gw_id').hide();
    	$('#td_gw_id_submit').hide();
    });

//######################
//# Setting AES
//######################
    $('#td_edit_aes').hide();
    $('#td_aes_submit').hide();
    $("#gw_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_aes').click(function() {	
    	$('#td_edit_aes').show();
    	$('#td_aes_submit').show();
    });
 
    $('#btn_aes_submit').click(function() {	
    	aes = $( "input:checked" ).val();
    	//alert(aes);
    	$.get("process.php", {aes: aes}, function(data){	
			//$('#gw_msg').html(data);
			//$('#gw_msg').show();
			$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000); 	
        });
    	
	if(aes != '') 
        	$('#aes_value').html(aes);
        $('#td_edit_aes').hide();
    	$('#td_aes_submit').hide();
    });

//######################
//# Setting downlink
//######################
    $('#td_edit_downlink').hide();
    $('#td_downlink_submit').hide();
    $("#gw_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_downlink').click(function() {	
    	$('#td_edit_downlink').show();
    	$('#td_downlink_submit').show();
    });
 
    $('#btn_downlink_submit').click(function() {	
    	downlink = $("#downlink_input").val();
    	$.get("process.php", {downlink: downlink}, function(data){	
			//$('#gw_msg').html(data);
			//$('#gw_msg').show();
			$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
        });

        	$('#downlink_value').html(downlink);
        	$("#downlink_input").val("");
       		 $('#td_edit_downlink').hide();
    		$('#td_downlink_submit').hide();
    });
    
//######################
//# Setting alert sms
//######################
    $('#td_edit_use_sms').hide();
    $('#td_use_sms_submit').hide();
    $("#alert_sms_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_use_sms').click(function() {	
    	$('#td_edit_use_sms').show();
    	$('#td_use_sms_submit').show();
    });
 
    $('#btn_use_sms_submit').click(function() {	
    	use_sms = $( "input:checked" ).val();
    	
    	$.get("process.php", {use_sms: use_sms}, function(data){	
			//$('#alert_sms_msg').html(data);
			//$('#alert_sms_msg').show();
			$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
        });
    	if(use_sms != '')	
        	$('#use_sms_value').html(use_sms);
        $('#td_edit_use_sms').hide();
    	$('#td_use_sms_submit').hide();
    });

//######################
//# Setting sms pin
//######################
    $('#td_edit_sms_pin').hide();
    $('#td_sms_pin_submit').hide();
    $("#alert_sms_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_sms_pin').click(function() {	
    	$('#td_edit_sms_pin').show();
    	$('#td_sms_pin_submit').show();
    });
 
    $('#btn_sms_pin_submit').click(function() {	
    	sms_pin = $("#sms_pin_input").val();
    	
	if (! /^\d{4}$/.test(sms_pin)){
		//$('#alert_sms_msg').html('<p><center><font color="red">Please enter a valid format!.</font></center></p>');
		//$('#alert_sms_msg').show();
		$("#gw_config_msg").html('<p><center><font color="red">Please enter a valid format!.</font></center></p>');
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
	}
	else{
		$.get("process.php", {sms_pin: sms_pin}, function(data){	
			//$('#alert_sms_msg').html(data);
			//$('#alert_sms_msg').show();
			$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
       		 });
    		
        	$('#sms_pin_value').html(sms_pin);
        	$("#sms_pin_input").val("");
       		 $('#td_edit_sms_pin').hide();
    		$('#td_sms_pin_submit').hide();
	}
    	
    });
    
//######################
//# Setting contact sms
//######################
    $('#td_edit_contact_sms').hide();
    $('#td_contact_sms_submit').hide();
    $("#alert_sms_msg").hide();
    
    $('#btn_edit_contact_sms').click(function() {	
    	$('#td_edit_contact_sms').show();
    	$('#td_contact_sms_submit').show();
    });
 
    $('#btn_contact_sms_submit').click(function() {	
    	contact_sms = $("#contact_sms_input").val();
    	
    	contact_sms = String(contact_sms);
		// array
		contacts = contact_sms.split(","); // For phone number validation : + & max 15 digits
		valid = true; i = 0;
		while(i < contacts.length && valid){
			if (! /^[\+]?[(]?[0-9]{3}[)]?[-\s\.]?[0-9]{3}[-\s\.]?[0-9]{4,9}$/.test(contacts[i]))
				valid = false;
			i++;
		}
		
		if(!valid){
			//$('#alert_sms_msg').html('<p><center><font color="red">Please enter a valid phone number.</font></center></p>');
			//$('#alert_sms_msg').show();
			$("#gw_config_msg").html('<p><center><font color="red">Please enter a valid phone number.</font></center></p>');
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
		}
		else{
			var contactsJSON = JSON.stringify(contacts);
			$.get("process.php", {contact_sms: contactsJSON}, function(data){
				//$('#alert_sms_msg').html(data);
				//$('#alert_sms_msg').show();
				$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
				$('#td_edit_contact_sms').hide();
    			$('#td_contact_sms_submit').hide();
        	});
    		
        	$('#contact_sms_value').html(contact_sms);
        	$("#contact_sms_input").val("");
        	$('#td_edit_contact_sms').hide();
    		$('#td_contact_sms_submit').hide();
		}
    	
    });


//######################
//# Setting alert mail
//######################
    $('#td_edit_use_mail').hide();
    $('#td_use_mail_submit').hide();
    $("#alert_mail_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_use_mail').click(function() {	
    	$('#td_edit_use_mail').show();
    	$('#td_use_mail_submit').show();
    });
 
    $('#btn_use_mail_submit').click(function() {	
    	use_mail = $( "input:checked" ).val();
    	
    	$.get("process.php", {use_mail: use_mail}, function(data){	
			//$('#alert_mail_msg').html(data);
			//$('#alert_mail_msg').show();
			$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
        });
    	
	if(use_mail != '')		
        	$('#use_mail_value').html(use_mail);
        $('#td_edit_use_mail').hide();
    	$('#td_use_mail_submit').hide();
    });

//######################
//# Setting mail account
//######################
    $('#td_edit_mail_from').hide();
    $('#td_mail_from_submit').hide();
    $("#alert_mail_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_mail_from').click(function() {	
    	$('#td_edit_mail_from').show();
    	$('#td_mail_from_submit').show();
    });
 
    $('#btn_mail_from_submit').click(function() {	
    	mail_from = $("#mail_from_input").val();
	
	if (! /^\w+([\.-]?\w+)*@\w+([\.-]?\w+)*(\.\w{2,3})+$/.test(mail_from)){
		//$('#alert_mail_msg').html('<p><center><font color="red">Please enter a valid email address.</font></center></p>');
		//$('#alert_mail_msg').show();
		$("#gw_config_msg").html('<p><center><font color="red">Please enter a valid email address.</font></center></p>');
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
	}
	else{
		$.get("process.php", {mail_from: mail_from}, function(data){	
			//$('#alert_mail_msg').html(data);
			//$('#alert_mail_msg').show();
			$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
       		 });
		
		$('#mail_from_value').html(mail_from);
	}
    		
        $("#mail_from_input").val("");
        $('#td_edit_mail_from').hide();
    	$('#td_mail_from_submit').hide();
    });

//######################
//# Setting mail password
//######################
    $('#td_edit_mail_passwd').hide();
    $('#td_mail_passwd_submit').hide();
    $("#alert_mail_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_mail_passwd').click(function() {	
    	$('#td_edit_mail_passwd').show();
    	$('#td_mail_passwd_submit').show();
    });
 
    $('#btn_mail_passwd_submit').click(function() {	
    	mail_passwd = $("#mail_passwd_input").val();
    	
    	$.get("process.php", {mail_passwd: mail_passwd}, function(data){	
			//$('#alert_mail_msg').html(data);
			//$('#alert_mail_msg').show();
			$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
        });
    	
	if(mail_passwd != '')	
        	$('#mail_passwd_value').html(mail_passwd);
        $("#mail_passwd_input").val("");
        $('#td_edit_mail_passwd').hide();
    	$('#td_mail_passwd_submit').hide();
    });
    
//######################
//# Setting mail server
//######################
    $('#td_edit_mail_server').hide();
    $('#td_mail_server_submit').hide();
    $("#alert_mail_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_mail_server').click(function() {	
    	$('#td_edit_mail_server').show();
    	$('#td_mail_server_submit').show();
    });
 
    $('#btn_mail_server_submit').click(function() {	
    	mail_server = $("#mail_server_input").val();
    	
	$.get("process.php", {mail_server: mail_server}, function(data){	
		//$('#alert_mail_msg').html(data);
		//$('#alert_mail_msg').show();
		$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
        });
	
	if(mail_server != '')	
		$('#mail_server_value').html(mail_server);
        $("#mail_server_input").val("");
        $('#td_edit_mail_server').hide();
    	$('#td_mail_server_submit').hide();

    });
    
//######################
//# Setting contact mail
//######################
    $('#td_edit_contact_mail').hide();
    $('#td_contact_mail_submit').hide();
    $("#alert_mail_msg").hide();
    
    $('#btn_edit_contact_mail').click(function() {	
    	$('#td_edit_contact_mail').show();
    	$('#td_contact_mail_submit').show();
    });
 
    $('#btn_contact_mail_submit').click(function() {	
    	contact_mail = $("#contact_mail_input").val();
    
		contact_mail = String(contact_mail);
		// array
		contacts = contact_mail.split(","); // For mail validation
		valid = true; i = 0;
		while(i < contacts.length && valid){
			if (! /^\w+([\.-]?\w+)*@\w+([\.-]?\w+)*(\.\w{2,3})+$/.test(contacts[i]))
				valid = false;
			i++;
		} 
		
		if(!valid){
			//$('#alert_mail_msg').html('<p><center><font color="red">Please enter a valid email address.</font></center></p>');
			//$('#alert_mail_msg').show();
			$("#gw_config_msg").html('<p><center><font color="red">Please enter a valid email address.</font></center></p>');
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
			$('#td_edit_contact_mail').hide();
    		$('#td_contact_mail_submit').hide();
		}
		else{
			contactsJSON = JSON.stringify(contacts);
			$.get("process.php", {contact_mail: contactsJSON}, function(data){		
				//$('#alert_mail_msg').html(data);
				//$('#alert_mail_msg').show();
				$("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
				$('#contact_mail_value').html(contact_mail);
        	});
    		
        	$("#contact_mail_input").val("");
        	$('#td_edit_contact_mail').hide();
    		$('#td_contact_mail_submit').hide();
		}
    });

//###########################
//# Setting ThingSpeak status
//###########################
    $('#td_edit_thingspeak_status').hide();
    $('#td_thingspeak_status_submit').hide();
    $("#thingspeak_status_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_thingspeak_status').click(function() {	
    	$('#td_edit_thingspeak_status').show();
    	$('#td_thingspeak_status_submit').show();
    });
 
    $('#btn_thingspeak_status_submit').click(function() {	
    	thingspeak_status = $( "input:checked" ).val();
    	
    	$.get("process.php", {thingspeak_status: thingspeak_status}, function(data){	
			//$('#thingspeak_status_msg').html(data);
			//$('#thingspeak_status_msg').show();
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	   		 setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
        });
    	/*
    	if($thingspeak_status == "true")
    		$('#thingspeak_status_value').html("Enable");
    	else
    		$('#thingspeak_status_value').html("Disable");
    	*/
	if(thingspeak_status != '')
        	$('#thingspeak_status_value').html(thingspeak_status);
        $('#td_edit_thingspeak_status').hide();
    	$('#td_thingspeak_status_submit').hide();
    });
    
//##############################
//# Setting ThingSpeak write key
//##############################
    $('#td_edit_write_key').hide();
    $('#td_write_key_submit').hide();
    $("#thingspeak_status_msg").hide();
    
    $('#btn_edit_write_key').click(function() {	
    	$('#td_edit_write_key').show();
    	$('#td_write_key_submit').show();
    });
 
    $('#write_key_submit').click(function() {	
    	write_key = $('#write_key_input').val();
    	
    	$.get("process.php", {write_key: write_key}, function(data){	
			//$('#thingspeak_status_msg').html(data);
			//$('#thingspeak_status_msg').show();
			$('#cloud_msg').html(data);
	    		//erase message after 5 seconds
	    		setTimeout(function() {
  				$('#cloud_msg').html("");
	   		 },5000);
        });
    	if(write_key != '')
        	$('#write_key_value').html(write_key);
        $('#write_key_input').val("");
        $('#td_edit_write_key').hide();
    	$('#td_write_key_submit').hide();
    });

//########################
//# Setting Waziup status
//########################
    $('#td_edit_waziup_status').hide();
    $('#td_waziup_status_submit').hide();
    $("#waziup_status_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_waziup_status').click(function() {	
    	$('#td_edit_waziup_status').show();
    	$('#td_waziup_status_submit').show();
    });
 
    $('#btn_waziup_status_submit').click(function() {	
    	waziup_status = $( "input:checked" ).val();
    	
    	$.get("process.php", {waziup_status: waziup_status}, function(data){	
			//$('#waziup_status_msg').html(data);
			//$('#waziup_status_msg').show();
			$('#cloud_msg').html(data);
	   		 //erase message after 5 seconds
	    		setTimeout(function() {
  				$('#cloud_msg').html("");
	    		},5000);
        });
        /*
    	if($waziup_status == "true")
    		$('#waziup_status_value').html("Enable");
    	else
    		$('#waziup_status_value').html("Disable");
    	*/
	if(waziup_status !='')
        	$('#waziup_status_value').html(waziup_status);
        $('#td_edit_waziup_status').hide();
    	$('#td_waziup_status_submit').hide();
    });

//##############################
//# Setting Waziup project name
//##############################
    $('#td_edit_project').hide();
    $('#td_project_submit').hide();
    $("#waziup_status_msg").hide();
    
    $('#btn_edit_project').click(function() {	
    	$('#td_edit_project').show();
    	$('#td_project_submit').show();
    });
 
    $('#project_submit').click(function() {	
    	project = $('#project_input').val();
    	
    	$.get("process.php", {project_name: project}, function(data){	
			//$('#waziup_status_msg').html(data);
			//$('#waziup_status_msg').show();
			$('#cloud_msg').html(data);
	   		 //erase message after 5 seconds
	    		setTimeout(function() {
  				$('#cloud_msg').html("");
	    		},5000);
        });
    	
	if(project !='')	
        	$('#project_value').html(project);
        $('#project_input').val("");
        $('#td_edit_project').hide();
    	$('#td_project_submit').hide();
    });

//###################################
//# Setting Waziup organization name
//###################################
    $('#td_edit_organization').hide();
    $('#td_organization_submit').hide();
    $("#waziup_status_msg").hide();
    
    $('#btn_edit_organization').click(function() {	
    	$('#td_edit_organization').show();
    	$('#td_organization_submit').show();
    });
 
    $('#organization_submit').click(function() {	
    	org = $('#organization_input').val();
    	
    	$.get("process.php", {organization_name: org}, function(data){	
			//$('#waziup_status_msg').html(data);
			//$('#waziup_status_msg').show();
			$('#cloud_msg').html(data);
	   		 //erase message after 5 seconds
	    		setTimeout(function() {
  				$('#cloud_msg').html("");
	    		},5000);
        });
    	
	if(org !='')	
        	$('#organization_value').html(org);
        $('#organization_input').val("");
        $('#td_edit_organization').hide();
    	$('#td_organization_submit').hide();
    });
    
//##############################
//# Setting Waziup service tree
//##############################
    $('#td_edit_service_tree').hide();
    $('#td_service_tree_submit').hide();
    $("#waziup_status_msg").hide();
    
    $('#btn_edit_service_tree').click(function() {	
    	$('#td_edit_service_tree').show();
    	$('#td_service_tree_submit').show();
    });
 
    $('#service_tree_submit').click(function() {	
    	serv = $('#service_tree_input').val();
    	//alert(serv);
    	$.get("process.php", {service_tree: serv}, function(data){	
			//$('#waziup_status_msg').html(data);
			//$('#waziup_status_msg').show();
			$('#cloud_msg').html(data);
	   		 //erase message after 5 seconds
	    		setTimeout(function() {
  				$('#cloud_msg').html("");
	    		},5000);
        });
    	
	if(serv !='')		
        	$('#service_tree_value').html(serv);
        $('#service_tree_input').val("");
        $('#td_edit_service_tree').hide();
    	$('#td_service_tree_submit').hide();
    });

    
//##############################
//# Setting Waziup auth token
//##############################
    $('#td_edit_auth_token').hide();
    $('#td_auth_token_submit').hide();
    $("#waziup_status_msg").hide();
    
    $('#btn_edit_auth_token').click(function() {	
    	$('#td_edit_auth_token').show();
    	$('#td_auth_token_submit').show();
    });
 
    $('#auth_token_submit').click(function() {	
    	auth_token = $('#auth_token_input').val();
    	//alert(serv);
    	$.get("process.php", {auth_token: auth_token}, function(data){	
			//$('#waziup_status_msg').html(data);
			//$('#waziup_status_msg').show();
			$('#cloud_msg').html(data);
	   		 //erase message after 5 seconds
	    		setTimeout(function() {
  				$('#cloud_msg').html("");
	    		},5000);
        });
    	
	if(auth_token !='')		
        	$('#auth_token_value').html(auth_token);
        $('#auth_token_input').val("");
        $('#td_edit_auth_token').hide();
    	$('#td_auth_token_submit').hide();
    });
    
//#######################
//# Setting access point
//#######################
	
	$('#btn_hostapd_form_reset').click(function() {	
    	$('#hostapd_form')[0].reset();
    	//$("#form_msg").html("");
	$("#system_msg").html("");
    });	

	$("#hostapd_form").submit(function(event){
        // Stop form from submitting normally
        event.preventDefault();
        
        /* Serialize the submitted form control values to be sent to the web server with the request */
        var formValues = $(this).serialize();
        
        // Send the form data using post
        $.post("process.php", formValues, function(data){
            // Display the returned data in browser
            //$("#form_msg").html(data);
	    $("#system_msg").html(data);
            //erase message after 5 seconds
	    setTimeout(function() {
  		$('#system_msg').html("");
  		window.location.reload();
	     },5000);
        });
        $('#hostapd_form')[0].reset();
    });

//#######################
//# Setting wifi client
//#######################
	
	$('#btn_wificlient_form_reset').click(function() {	
    	$('#wificlient_form')[0].reset();
    	//$("#form_msg").html("");
	$("#system_msg").html("");
    });	

	$("#wificlient_form").submit(function(event){
        // Stop form from submitting normally
        event.preventDefault();
        
        /* Serialize the submitted form control values to be sent to the web server with the request */
        var formValues = $(this).serialize();
        
        // Send the form data using post
        $.post("process.php", formValues, function(data){
            // Display the returned data in browser
            //$("#form_msg").html(data);
	    $("#system_msg").html(data);
            //erase message after 5 seconds
	    setTimeout(function() {
  		$('#system_msg').html("");
	     },5000);
        });
        $('#wificlient_form')[0].reset();
        
    });

//#######################
//# Setting AP mode
//#######################

	$('#btn_apmode').click(function() {
        $.post($(this).attr('href'), function (res) { 
	     $('#system_msg').html(res);
	    //erase message after 5 seconds
	    setTimeout(function() {
  		$('#system_msg').html("");
	    },5000);
        });
    });

//#######################
//# Setting AP mode now
//#######################

	$('#btn_apmodenow').click(function() {
        $.post($(this).attr('href'), function (res) { 
	     $('#system_msg').html(res);
	    //erase message after 5 seconds
	    setTimeout(function() {
  		$('#system_msg').html("");
	    },5000);
        });
    });
                
//#######################
//# Downlink request form
//#######################
	
   $('#btn_downlink_form_reset').click(function() {	
    	$('#downlink_form')[0].reset();
    	$("#downlink_form_msg").html("");
   });	

   $("#downlink_form").submit(function(event){
        // Stop form from submitting normally
        event.preventDefault();
        
        /* Serialize the submitted form control values to be sent to the web server with the request */
        var formValues = $(this).serialize();
        
        // Send the form data using post
        $.post("process.php", formValues, function(data){
            // Display the returned data in browser
            //$("#downlink_form_msg").html(data);
	    $("#gw_config_msg").html(data);
            			//erase message after 5 seconds
	   			setTimeout(function() {
  					$('#gw_config_msg').html("");
	     			},5000);
        });
        $('#downlink_form')[0].reset();
        
    });

//#######################
//# Setting profile
//#######################
	
	$('#btn_profile_form_reset').click(function() {	
    	$('#profile_form')[0].reset();
	$("system_msg").html("");
    });	

	$("#profile_form").submit(function(event){
        // Stop form from submitting normally
        event.preventDefault();
        
        /* Serialize the submitted form control values to be sent to the web server with the request */
        var formValues = $(this).serialize();
        
        // Send the form data using post
        $.post("process.php", formValues, function(data){
            // Display the returned data in browser
	    $("#system_msg").html(data);
            //erase message after 5 seconds
	    setTimeout(function() {
  		$('#system_msg').html("");
	     },5000);
        });
        $('#profile_form')[0].reset();
        
    });

});


