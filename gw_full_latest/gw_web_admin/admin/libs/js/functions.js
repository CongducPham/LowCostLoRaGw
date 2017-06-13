// shorter syntax for document.ready
$(function () {

//######################
//# Reboot
//######################
	$('#btn_reboot').click(function() {
        $.post($(this).attr('href'), function (res) { 
            // handle the response (*) by storing the message into the DIV#message
            //$('#msg').html(res);
        });
    });

//######################
//# Shutdown
//######################
	$('#btn_shutdown').click(function() {
        $.post($(this).attr('href'), function (res) { 
            // handle the response (*) by storing the message into the DIV#message
            //$('#msg').html(res);
        });
    });

//######################
//# Gateway: full update
//######################
	$('#btn_gw_full_update').click(function() {
        $.post($(this).attr('href'), function (res) { 
            // handle the response (*) by storing the message into the DIV#message
            $('#gw_full_update_msg').html(res);
        });
    });
    
//###########################
//# Gateway: install
//###########################
	$('#btn_gw_new_install').click(function() {
        $.post($(this).attr('href'), function (res) { 
            // handle the response (*) by storing the message into the DIV#message
            $('#gw_new_install_msg').html(res);
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
            $("#gw_update_form_msg").html(data);
        });
        $('#gw_update_form')[0].reset();
        
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
            $("#thingspeak_form_msg").html(data);
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
    	$mode = $('#mode_select').val();
    	
    	if($mode == ''){
    		alert("Please select one mode!");
    	}
    	else{ 
    		$.get("process.php", {mode: $mode}, function(data){
            	// Display the returned data in browser
            	//$("#mode_test").html(data);
            	//alert(data);
				$('#radio_msg').html(data);
				$('#radio_msg').show();
        	});
    		
        	$('#mode_value').html($mode);
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
    	$band = $('#band_freq_select').val();
    	
    	if($band == ''){
    		//alert("Please select one frequency band!");
    		$('#freq_select').html("");
    	}
    	else if($band == "433MHz"){
    		$('#freq_select').html("<option>433.3</option><option>433.6</option><option>433.9</option><option>434.3</option>");
    	}
    	else if($band == "868MHz"){
    		$('#freq_select').html("<option>863.2</option><option>863.5</option><option>863.8</option>"+
    								"<option>864.1</option><option>864.4</option><option>864.7</option>"+
    								"<option selected>865.2</option><option>865.5</option><option>865.8</option>"+
    								"<option>865.2</option><option>865.5</option><option>865.8</option>"+
    								"<option>866.1</option><option>866.4</option><option>866.7</option>"+
    								"<option>867.0</option><option>868.0</option><option>868.1</option>");
    	}
    	else{ // 915 MHz
    		$('#freq_select').html("<option>903.08</option><option>905.24</option><option>907.40</option>"+
    								"<option>909.56</option><option>911.72</option><option selected>913.88</option>"+
    								"<option>915.0</option><option>916.04</option><option>918.20</option>"+
    								"<option>922.52</option><option>924.68</option><option>926.84</option>");
    	}
    	
    	
    });
    
    
    $('#freq_submit').click(function() {	
    	
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
    	}
    	
    });
   										   
    
//######################
//# Setting Gatewway ID
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
    	$gw_id = $('#gw_id_input').val();
    	
    	$.get("process.php", {gateway_ID: $gw_id}, function(data){	
			$('#gw_msg').html(data);
			$('#gw_msg').show();
        });
    		
        $('#gw_id_value').html($gw_id);
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
    	$aes = $( "input:checked" ).val();
    	
    	$.get("process.php", {aes: $aes}, function(data){	
			$('#gw_msg').html(data);
			$('#gw_msg').show();
        });
    		
        $('#aes_value').html($aes);
        $('#td_edit_aes').hide();
    	$('#td_aes_submit').hide();
    });

//######################
//# Setting alert sms
//######################
    $('#td_edit_use_sms').hide();
    $('#td_use_sms_submit').hide();
    $("#alert_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_use_sms').click(function() {	
    	$('#td_edit_use_sms').show();
    	$('#td_use_sms_submit').show();
    });
 
    $('#btn_use_sms_submit').click(function() {	
    	$use_sms = $( "input:checked" ).val();
    	
    	$.get("process.php", {use_sms: $use_sms}, function(data){	
			$('#alert_msg').html(data);
			$('#alert_msg').show();
        });
    		
        $('#use_sms_value').html($use_sms);
        $('#td_edit_use_sms').hide();
    	$('#td_use_sms_submit').hide();
    });
    
//######################
//# Setting alert mail
//######################
    $('#td_edit_use_mail').hide();
    $('#td_use_mail_submit').hide();
    $("#alert_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_use_mail').click(function() {	
    	$('#td_edit_use_mail').show();
    	$('#td_use_mail_submit').show();
    });
 
    $('#btn_use_mail_submit').click(function() {	
    	$use_mail = $( "input:checked" ).val();
    	
    	$.get("process.php", {use_mail: $use_mail}, function(data){	
			$('#alert_msg').html(data);
			$('#alert_msg').show();
        });
    		
        $('#use_mail_value').html($use_mail);
        $('#td_edit_use_mail').hide();
    	$('#td_use_mail_submit').hide();
    });

//######################
//# Setting ThingSpeak status
//######################
    $('#td_edit_thingspeak_status').hide();
    $('#td_thingspeak_status_submit').hide();
    $("#thingspeak_status_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_thingspeak_status').click(function() {	
    	$('#td_edit_thingspeak_status').show();
    	$('#td_thingspeak_status_submit').show();
    });
 
    $('#btn_thingspeak_status_submit').click(function() {	
    	$thingspeak_status = $( "input:checked" ).val();
    	
    	$.get("process.php", {thingspeak_status: $thingspeak_status}, function(data){	
			$('#thingspeak_status_msg').html(data);
			$('#thingspeak_status_msg').show();
        });
    	/*
    	if($thingspeak_status == "true")
    		$('#thingspeak_status_value').html("Enable");
    	else
    		$('#thingspeak_status_value').html("Disable");
    	*/
        $('#thingspeak_status_value').html($thingspeak_status);
        $('#td_edit_thingspeak_status').hide();
    	$('#td_thingspeak_status_submit').hide();
    });
    
//######################
//# Setting Waziup status
//######################
    $('#td_edit_waziup_status').hide();
    $('#td_waziup_status_submit').hide();
    $("#waziup_status_msg").hide();
    //$("#div_mode_select").hide();
    
    $('#btn_edit_waziup_status').click(function() {	
    	$('#td_edit_waziup_status').show();
    	$('#td_waziup_status_submit').show();
    });
 
    $('#btn_waziup_status_submit').click(function() {	
    	$waziup_status = $( "input:checked" ).val();
    	
    	$.get("process.php", {waziup_status: $waziup_status}, function(data){	
			$('#waziup_status_msg').html(data);
			$('#waziup_status_msg').show();
        });
        /*
    	if($waziup_status == "true")
    		$('#waziup_status_value').html("Enable");
    	else
    		$('#waziup_status_value').html("Disable");
    	*/
        $('#waziup_status_value').html($waziup_status);
        $('#td_edit_waziup_status').hide();
    	$('#td_waziup_status_submit').hide();
    });
  
//#######################
//# Setting access point
//#######################
	
	$('#btn_hostapd_form_reset').click(function() {	
    	$('#hostapd_form')[0].reset();
    	$("#form_msg").html("");
    });	

	$("#hostapd_form").submit(function(event){
        // Stop form from submitting normally
        event.preventDefault();
        
        /* Serialize the submitted form control values to be sent to the web server with the request */
        var formValues = $(this).serialize();
        
        // Send the form data using post
        $.post("process.php", formValues, function(data){
            // Display the returned data in browser
            $("#form_msg").html(data);
        });
        $('#hostapd_form')[0].reset();
        
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
            $("#thingspeak_form_msg").html(data);
        });
        $('#thingspeak_form')[0].reset();
        
    });
    
//#######################
//# Setting Waziup
//#######################
	
	$('#btn_waziup_form_reset').click(function() {	
    	$('#waziup_form')[0].reset();
    	$("#waziup_form_msg").html("");
    });	

	$("#waziup_form").submit(function(event){
        // Stop form from submitting normally
        event.preventDefault();
        
        /* Serialize the submitted form control values to be sent to the web server with the request */
        var formValues = $(this).serialize();
        
        // Send the form data using post
        $.post("process.php", formValues, function(data){
            // Display the returned data in browser
            $("#waziup_form_msg").html(data);
        });
        $('#waziup_form')[0].reset();
        
    });

});
