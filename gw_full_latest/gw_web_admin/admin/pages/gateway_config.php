<?php
include_once '../libs/php/functions.php';

// begin our session
session_start();

// check if the user is logged out
if(!isset($_SESSION['username'])){
                header('Location: login.php');
                exit();
}

$radio_conf= null; $gw_conf= null; $alert_conf = null;
process_gw_conf_json($radio_conf, $gw_conf, $alert_conf);

$clouds = null; $encrypted_clouds= null; $lorawan_encrypted_clouds = null;
process_clouds_json($clouds, $encrypted_clouds, $lorawan_encrypted_clouds);

$key_clouds = process_key_clouds();

$maxAddr = 255;
$low_level_status_interval = 11;

if (is_file("/lib/systemd/system/chirpstack-network-server.service")) { 
	ob_start();
	system("systemctl is-active chirpstack-network-server.service | grep inactive");
	$chirpstack_not_active=ob_get_contents(); 
	ob_clean();
}
else
	$chirpstack_not_active="unknown";
	
$is_sx1301=exec('egrep "start_[ul]p[lf]_pprocessing_gw.sh" /etc/rc.local');	

require 'header.php';
?>
            <div class="navbar-default sidebar" role="navigation">
                <div class="sidebar-nav navbar-collapse">
                    <ul class="nav" id="side-menu">
                        </br>
                        
                        <li>
                            <a href="cloud.php"><i class="fa fa-cloud"></i> Clouds</a>
                        </li>
                        <li>
                            <a href="gateway_update.php"><i class="fa fa-upload"></i> Gateway Update</a>
                        </li>
                        <li>
                            <a href="system.php"><i class="fa fa-linux"></i> System</a>
                        </li>
                        <li>
                            <a href="jupyter.php"><img src="jupyter-logo.svg" width="15" height="14"> Jupyter</a>
                        </li>                        
                    </ul>
                </div>
                <!-- /.sidebar-collapse -->
            </div>
            <!-- /.navbar-static-side -->
        </nav>

        <div id="page-wrapper">
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Gateway configuration</h1>
                </div>
            </div>
            <!-- /.row -->
          
            <div class="panel panel-default">
                        <div class="panel-heading">
                           <div id="gw_config_msg"></div>
                        </div>
                        <!-- /.panel-heading -->
                        <div class="panel-body">
                            <!-- Nav tabs -->
                            <ul class="nav nav-pills">
                                <li class="active"><a href="#radio_conf-pills" data-toggle="tab">Radio</a>
                                </li>
                                <li><a href="#gw_conf-pills" data-toggle="tab">Gateway</a>
                                </li>
                                <li><a href="#netserv-pills" data-toggle="tab">Network Server</a>
                                </li>                                
                                <li><a href="#alert_mail-pills" data-toggle="tab">Alert Mail</a>
                                </li>
                                <li><a href="#alert_sms-pills" data-toggle="tab">Alert SMS</a>
                                </li>
                                <li><a href="#downlink-pills" data-toggle="tab">Downlink Request</a>
                                </li>
                                <li><a href="#copy-log-pills" data-toggle="tab">Get post-processing.log file</a>
                                </li>                                                               
                            </ul>

							</br>
                            							
            				<p>&nbsp;&nbsp;&nbsp;&nbsp;After changing gateway parameters, you need to reboot for changes to take effect.</p>
							<?php
								$date=date('Y-m-d\TH:i:s');
								echo '<p>&nbsp;&nbsp;&nbsp;&nbsp;Date/Time: ';
								echo $date;
								echo '</p>';
							?>	
										            				
                            <!-- Tab panes -->
                            <div class="tab-content">
                            	<div class="tab-pane fade in active" id="radio_conf-pills">

										<?php 									
											if ($is_sx1301=='')
											{
												echo '<p>&nbsp;&nbsp;&nbsp;&nbsp;Radio configuration file is for single channel radio</p>';
									
												ob_start(); 
												system("grep -a 'Low-level gw status ON' /home/pi/lora_gateway/log/post-processing.log | tail -1 | cut -d '.' -f1");
												$low_level=ob_get_contents(); 
												ob_clean();
												if ($low_level=='') {
													echo '<p>&nbsp;&nbsp;&nbsp;&nbsp;<font color="red"><b>no low-level status</b></font></p>';					
												}
												else {
													$date=str_replace("T", " ", $date, $count);
													$datetime1 = new DateTime($date);
													$low_level_1=str_replace("T", " ", $low_level, $count);
													$datetime2 = new DateTime($low_level_1);
													$interval = $datetime1->diff($datetime2);
											
													$interval_minute=intval($interval->format('%i'));
											
													//we have to check month, day and hour because minute is only between 1..59
													if (intval($interval->format('%m'))>0 || intval($interval->format('%d'))>0 || intval($interval->format('%h'))>0) {
														$interval_minute=$low_level_status_interval+1;
													}
											
													echo '<p>&nbsp;&nbsp;&nbsp;&nbsp;last low-level status: ';
											
													if ($interval_minute > $low_level_status_interval) {
														echo '<font color="red"><b>';
													}
													else {
														echo '<font color="green"><b>';
													}
				
													echo $low_level;
													echo $interval->format(' %mm-%dd-%hh-%imin from current date');	
													echo '</b></font></p>';				
												}																							
											}
											else {
												echo '<p>&nbsp;&nbsp;&nbsp;&nbsp;Radio configuration file is for SX1301-based multi-channel concentrator [<a href="../log/global_conf.json">global_conf.json</a>][<a href="../log/local_conf.json">local_conf.json</a>]</p>';
												echo '<div style="margin-left: 1em;">';
													echo '<form id="download_sx1301_conf_form" role="form">';
														echo '<fieldset>';
															echo '<div class="form-group">';
																echo '<label>Enter the URL of a valid <tt>global_conf.json</tt> file if you want to replace the existing one</label>';
																echo '<input class="form-control" placeholder="" name="sx1301_conf_file_name_url" type="text" value="" autofocus>';
															echo '</div>';
															echo '<p>e.g. EU863-870 from TTN [<a href="https://raw.githubusercontent.com/TheThingsNetwork/gateway-conf/master/EU-global_conf.json">copy link</a>]';
															echo ' or from RAK [<a href="https://raw.githubusercontent.com/RAKWireless/rak_common_for_gateway/master/lora/rak2245/global_conf/global_conf.eu_863_870.json">copy link</a>]&nbsp;&nbsp;&nbsp;&nbsp;';
															echo '<button type="submit" class="btn btn-primary">Download</button></p></br>';
														echo '</fieldset>';
													echo '</form>';
												echo '</div>';												
											}
											
											ob_start(); 
											system("grep -a '\+\+\+ rxlora' /home/pi/lora_gateway/log/post-processing.log | tail -1");
											$rx=ob_get_contents(); 
											ob_clean();
											if ($rx=='') {
												echo '<p>&nbsp;&nbsp;&nbsp;&nbsp;<font color="red"><b>no rx found</b></font>';					
											}
											else {
												echo '&nbsp;&nbsp;&nbsp;&nbsp;last rx: <font color="green"><b>';
												echo $rx;
												echo '</b></font></p>';					
											}												
										?>
									
                                    </br>
                                    
                                    <div id="radio_msg"></div>
                                    
                                    <div class="col-md-10 col-md-offset-0">
                                      <div class="table-responsive">
										<table class="table table-striped table-bordered table-hover">
   										  <thead>
    								       
    									 </thead>
										<tbody>
										   <tr>
    									    <td><a href="#" class="my_tooltip" data-toggle="tooltip" title="mode 1 (SF12BW125)<br/>mode 2 (SF12BW250)<br/>mode 3 (SF10BW125)<br/>mode 4 (SF12BW500)<br/>mode 5 (SF10BW250)<br/>mode 6 (SF11BW500)<br/>mode 7 (SF09BW250)<br/>mode 8 (SF09BW500)<br/>mode 9 (SF08BW500)<br/>mode 10 (SF07BW500)<br/>mode 11 (LoRaWAN)">Mode</a>
    									    <?php
    								    		if ($radio_conf['mode']==11) {
    								    			echo " <b>[LoRaWAN]</b>";
    								    		if ($gw_conf['raw']==false)
    								    			echo '<font color="red">[set raw format to true]</font>';
    								    	}
    								    ?>
    									    </td>
    										<td id="mode_value"><?php echo $radio_conf['mode']; ?></td>
    										<td align="right"><button id="btn_edit_mode" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										    <td id="td_edit_mode">
   										    	<div id="div_mode_select" class="form-group">
                                            		<label>Select a mode</label>
                                            		<select id="mode_select" class="form-control">
                                                		<option>-1</option> <option selected>1</option> <option>2</option> <option>3</option> <option>4</option>
                                                		<option>5</option> <option>6</option> <option>7</option> <option>8</option>
                                                		<option>9</option> <option>10</option>
                                           			</select>
                                        		</div>
                                        	</td> 
   										    <td id="mode_submit" align="right">
   										    		<button id="btn_edit_mode" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>	
   										   </tr>

										   <tr>
    									    <td>Spreading Factor</td>
    										<td id="sf_value"><?php echo $radio_conf['sf']; ?></td>
    										<td align="right"><button id="btn_edit_sf" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										    <td id="td_edit_sf">
   										    	<div id="div_sf_select" class="form-group">
                                            		<label>Select a SF</label>
                                            		<select id="sf_select" class="form-control">
                                                		<option>7</option> <option>8</option>
                                                		<option>9</option> <option>10</option> <option>11</option> <option selected>12</option>
                                           			</select>
                                        		</div>
                                        	</td> 
   										    <td id="sf_submit" align="right">
   										    		<button id="btn_edit_sf" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>	
   										   </tr>
   										      					
   										   <tr>
    									    <td>Frequency</td>
    										<td id="freq_value"><?php echo $radio_conf['freq']; ?></td>
    										<td align="right"><button id="btn_edit_freq" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_freq">
   										    	<div class="form-group">
                                            		<label>ISM Band </label>
                                            		<select id="band_freq_select" class="form-control">
                                                		<option></option> <option>-1</option> <option>433MHz</option> <option>868MHz</option> <option>915MHz</option> 
                                           			</select>
                                           			
                                           			<label>Frequency</label>
                                            		<select id="freq_select" class="form-control">
                                                		
                                           			</select>
                                        		</div>
                                        	</td>
                                        	<td id="freq_submit" align="right"><button id="btn_edit_mode" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button></td> 
   										   </tr>

   										   <tr>
    									    <td>PA_BOOST</td>
    										<td id="paboost_value">
    											<?php
    												$current_paboost=exec('egrep "^CFLAGS.*BOOST" /home/pi/lora_gateway/radio.makefile');
    												
    												if ($current_paboost=='')
													{
    													//echo "Disabled";
    													$current_paboost=false;    
													}
													else {
   					 									//echo "Enabled";
   					 									$current_paboost=true;   
													}
    											?>  										
    										</td>
    										
											<td align="right">
											<input type="checkbox" id="paboost_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if($current_paboost) echo "checked";?>>
											</td>
			
											<!-- <td align="right"><button id="btn_edit_paboost" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td> -->
											
   										   	<td id="td_edit_paboost">
   										    	<div id="div_paboost_options" class="form-group">
                                            		<label></label>
                                           			<div class="radio">
                                           			<fieldset id="paboost_group" >
                                                		<label>
                                                    		<input type="radio" name="paboost_group" id="paboost_true" value="Enabled" <?php if($current_paboost) echo "checked";?> >Enabled
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="paboost_group" id="paboost_false" value="Disabled" <?php if(!$current_paboost) echo "checked";?> >Disabled
                                                		</label>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_paboost_submit" align="right">
   										    		<button id="btn_paboost_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>    									    
   										   </tr>   										   
   										   
										</tbody>
    								  </table>
    								  <p><b>Mode=11 will indicate LoRaWAN mode. Use "Configure for LoRaWAN" feature in Gateway tab to set for LoRaWAN mode.</b></p>
    								  <p>For single-channel gateways, the default LoRaWAN mode means SF12BW125 and <a href="#" class="my_tooltip" data-toggle="tooltip" title="LoRaWAN defines a sync word of 0x34 for a public LoRaWAN networks. A typical LoRaWAN device will therefore use this sync word value.">sync word</a> 0x34 . In this mode you can change the Spreading Factor SF.</p>
    								  <p>Change frequency for a single-channel gateway if needed. Leave frequency as -1 to use <a href="#" class="my_tooltip" data-toggle="tooltip" title="865.2MHz for BAND868, 913.88MHz for BAND900 and 433.3 for BAND433">default values</a> (for LoRaWAN mode: 868.1MHz for BAND868, 923.2MHz for BAND900 and 433.175 for BAND433).</p>
    								  <p>PA_BOOST is required for some radio modules such as inAir9B, RFM92W, RFM95W, NiceRF LoRa1276. After changing the PA_BOOST settings, run <b>Gateway Update/Basic config</b> to recompile the low-level gateway program.</p> 
    							     </div>
    							     
    							  </div>
    							                           		
                                </div>
                                <!-- tab-pane -->
                                
                                <div class="tab-pane fade" id="gw_conf-pills">
                                	<div class="col-md-10 col-md-offset-0">
                                	
									<form id="gw_lorawan_conf_form" role="form">
										<fieldset>
											<a href="#" class="my_tooltip" data-toggle="tooltip" title="set mode to 11, raw format to true, aes_lorawan to false, status interval to 600s (5min) if interval is 0, disable non-LoRaWAN downlink timer, enable downlink process for LoRaWAN and enable/disable LoRaWAN cloud as selected"><button  id="btn_gw_lorawan_conf" type="button" class="btn btn-primary">Configure for LoRaWAN</button></a>
											<input type="checkbox" id="gw_lorawan_conf_ttn_checkbox" name="gw_lorawan_conf_ttn_checkbox" value="true" <?php if (get_cloud_status($lorawan_encrypted_clouds, "python CloudTTN.py")) echo "checked";?>>TTN cloud</input>
											<?php
											if (is_file("/lib/systemd/system/chirpstack-network-server.service")) {
												echo '<input type="checkbox" id="gw_lorawan_conf_chirpstack_checkbox" name="gw_lorawan_conf_chirpstack_checkbox" value="true" ';
												if (get_cloud_status($lorawan_encrypted_clouds, "python CloudChirpStack.py")) 
													echo 'checked';
												echo '>ChirpStack cloud</input>';
											}
											?>
										</fieldset>
									</form>
									
									<?php
										if (get_cloud_status($lorawan_encrypted_clouds, "python CloudChirpStack.py") or get_cloud_status($lorawan_encrypted_clouds, "python CloudTTN.py"))
											$cloud_lorawan_enabled=true;
										else
											$cloud_lorawan_enabled=false;
											
    								 	if ($cloud_lorawan_enabled) {
											if ($is_sx1301=='') {
												echo '<p><font color="green">You have a single-channel gateway. </font>';

												if ($radio_conf['mode']==11)
													echo '<font color="green">Radio mode is for LoRaWAN. You can have <a href="#" class="my_tooltip" data-toggle="tooltip" title="Join-request for OTAA must use LoRa uplink setting.">limited</a> LoRaWAN support.</font></p>';
												else
													echo '<font color="orange">Radio mode is not for LoRaWAN. You can configure for <a href="#" class="my_tooltip" data-toggle="tooltip" title="Join-request for OTAA must use LoRa uplink setting.">limited</a> LoRaWAN support</font></p>';     								 				
											}
											else {
												echo '<p><font color="green">You have a multi-channel gateway. </font>';
												if ($radio_conf['mode']==11)
													echo '<font color="green">Radio mode is for LoRaWAN. You can have <a href="#" class="my_tooltip" data-toggle="tooltip" title="All downlink messages will be handled by Semtech lora_pkt_fwd">full</a> LoRaWAN support.</font></p>';
												else
													echo '<font color="orange">Radio mode is not for LoRaWAN. You can configure for <a href="#" class="my_tooltip" data-toggle="tooltip" title="All downlink messages will be handled by Semtech lora_pkt_fwd">full</a> LoRaWAN support.</font></p>';
											}
																								
										if (get_cloud_status($lorawan_encrypted_clouds, "python CloudChirpStack.py") and $chirpstack_not_active!='')
											echo '<p><font color="orange">You are pushing to ChirpStack but there is no local ChirpStack service. Be sure to have a remote ChirpStack Network Server available.</font></p>';
										}
										else {
											echo '<p><font color="orange">You have no LoRaWAN cloud enabled.</font></p>'; 
										}								
													
									?>				
													                             
                                    </br>
                                    <!-- <h4>Gateway settings</h4></br> -->
                                    <div id="gw_msg"></div>
                                    <!-- Tab panes -->

                                      <div class="table-responsive">
										<table class="table table-striped table-bordered table-hover">
   										  <thead>
    								       
    									 </thead>
										 <tbody>
										   <tr>
    									    <td>Gateway ID</td>
    										<td id="gw_id_value"><?php echo $gw_conf['gateway_ID']; ?></td>
    										<td align="right"><button id="btn_edit_gw_id" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_gw_id">
   										    	<div id="div_update_gw_id" class="form-group">
                                            		<label>Gateway ID</label>
													<input id="gw_id_input" class="form-control" placeholder="New value" type="text" value="" autofocus>
													<p><font color="red">WARNING: the gateway id is normally derived from the MAC address of the gateway. When you run Update/Basic config the gateway id will be automatically determined so it is not recommended to manually edit the gateway id.</font></p>
                                        		</div>
                                        	</td> 
   										    <td id="td_gw_id_submit" align="right">
   										    		<button id="gw_id_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   
   										   </tr>
    									    <td>Gateway ID MD5 hashed</td>
    										<td id="gw_id_md5_value">
    											<?php
    												$gw_id_md5=exec('cat /home/pi/lora_gateway/gateway_id.md5');
    												
    												if ($gw_id_md5=='')
													{
    													echo "not defined";    
													}
													else {
   					 									echo $gw_id_md5;   
													}
    											?>     										
    										</td>
											<td align="right">not editable </td>
   										   </tr>   										   
	
   										   <!-- ajout de l'adresse ip et de l'adresse mac -->
   										   <tr>
   										   <td>IP address</td>
   										   <td id="ip_address_value">
   										   		<!--
   										   		<?php 
													ob_start(); 
													system("ifconfig eth0");
													$mycom=ob_get_contents(); 
													ob_clean(); 
													$findme = "inet addr"; 
													$pip = strpos($mycom, $findme); 
													$next = "Bcast";
													$nextword = strpos($mycom, $next);
													if ($nextword === false) {
														ob_start();
														system("ifconfig ppp0");
														$mycom=ob_get_contents(); 
														ob_clean(); 
														$findme = "inet addr"; 
														$pip = strpos($mycom, $findme); 
														$next = "P-t-P";
														$nextword = strpos($mycom, $next);
														if ($nextword === false) {
															ob_start();
															system("ifconfig wlan0");
															$mycom=ob_get_contents(); 
															ob_clean(); 
															$findme = "inet addr"; 
															$pip = strpos($mycom, $findme); 
															$next = "Bcast";
															$nextword = strpos($mycom, $next);
															if ($nextword === false) {
																echo "cannot be determined";
															}
															else {
																$length = $nextword - $pip - strlen($findme) - strlen($next) +3;
																$ip=substr($mycom,($pip+(strlen($findme)+1)),$length); 
																echo "wlan0: $ip";
															}														
														}
														else {
															$length = $nextword - $pip - strlen($findme) - strlen($next) +3;
															$ip=substr($mycom,($pip+(strlen($findme)+1)),$length); 
															echo "ppp0: $ip";														
														}													
													}
													else {
														$length = $nextword - $pip - strlen($findme) - strlen($next) +3;
														$ip=substr($mycom,($pip+(strlen($findme)+1)),$length); 
														echo "eth0: $ip";													
													} 
												?>
												-->
   										   		<?php 
													ob_start(); 
													system("hostname -I | cut -d ' ' -f1");
													$ip=ob_get_contents(); 
													ob_clean(); 
													echo "$ip";
												?>												
   										   </td>
   										   <td align="right">not editable</td>
   										   </tr>	
   										   
   										   <tr>
   										   	<td>MAC addresss</td>
   										   	<td id="mac_address_value">
   										   		<!--
   										   		<?php 
													ob_start(); 
													system("ifconfig eth0"); 
													$mycom=ob_get_contents(); 
													ob_clean(); 
													$findme = "HWaddr"; 
													$pmac = strpos($mycom, $findme); 
													$mac=substr($mycom,($pmac+7),17); 
													echo "eth0: $mac"; 
												?>
												-->
   										   		<?php 
													ob_start(); 
													system("ifconfig eth0 | grep -o -E '([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}'");
													$mac=ob_get_contents(); 
													ob_clean(); 
													echo "eth0: $mac"; 
												?>												
   										   	</td> 
   										   	<td align="right">not editable</td>
   										   </tr>	
   										   
   										   <tr>
   										   		<td>GPS coordinates</td>
   										   		<td>
   										   			<div id="latitude_gw_conf">
   										   				<?php echo  $gw_conf['ref_latitude'];?>
   										   			</div>
   										   			<div id="longitude_gw_conf" >
   										   			 	<?php echo  $gw_conf['ref_longitude'];?>
   										   			</div>
   										   			<!--
													<div id="div_select_display_format" class="btn-group" data-toggle="buttons">
   										   				<div id="div_update_display_format" class="form-group">
   										   				<label>
                                                    		<input type="radio" name="optionsRadios" id="display_format_dd" value="dd" checked> Decimal degree
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="display_format_dms" value="dms" > Degree, minute, second
                                                		</label>
                                                		</div>
   										   			</div>
													-->
   										   			<div id="latitude_value" class="display:inline">
   														<?php echo  "Latitude : " . $gw_conf['ref_latitude'];?>
   										   			</div>
   										   			<div id="longitude_value" class="display:inline	">
   										   			 	<?php echo  "Longitude : " . $gw_conf['ref_longitude'];?>
   										   			</div>
   										   			
   										   			
   										   		</td>
   										   		<td align="right" id="td_settings_gw_position">
   										   			<button id="btn_edit_gw_position" type="button" class="btn btn-primary">
   										   				<span class="fa fa-edit"></span>
   										   			</button>
   										   		</td>
   										   	</tr>
   										   	<tr>
   										   		<td id="td_edit_gw_position">
   										   			<div id="div_select_format_position" class="btn-group" data-toggle="buttons">
   										   				<div id="div_update_format_position" class="form-group">
   										   				<label>
                                                    		<input type="radio" name="optionsRadios" id="format_dd" value="dd" checked> Decimal degree
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="format_dms" value="dms" > Degree, minute, second
                                                		</label>
                                                		</div>
								<div id="div_info_format" class="alert alert-danger"></div>
   										   			</div>
   										   		</td>
   										   		<td id="td_format_position_dd">
   										   			<div id="div_update_dd_position" class="form-group">
   										   				<div class="radio">
   										   				<label>Latitude</label>
   										   				<input id="latitude_dd_input" class="form-control" placeholder="43.2951" name="latitude" type="number_dd" value=""  autofocus>
   										   				</br>
   										   				<label>Longitude</label>
   										   				<input id="longitude_dd_input" class="form-control" placeholder="-0.3707970000000387" name="longitude_dd" type="number" value="" >
   										 				</br>		
   										 				</div>	
   										   			</div>
													
   										   		</td>
   										   		<td id="td_format_position_dms">
   										   			<div id="div_update_dms_position" class="form-group">				
   										   				<label>Latitude</label>
   										   				<div class="radio" align="left">
   										   				<fieldset id="latitude_group" >	
   										   					<label>
   										   					<input type="radio" name="latitude_group" id="latitude_north" value="N" checked>N
   										   					</label>
   										   					<label>
   										   					<input type="radio" name="latitude_group" id="latitude_south" value="S" >S
   										   					</label>
   										   					</br>
   										   				</fieldset>
   										   				</div>
   										   				<input id="latitude_degree_input" class="form-control" placeholder="43" name="latitude_degree" type="number" value="">
   										   				<input id="latitude_minute_input" class="form-control" placeholder="17" name="latitude_minute" type="number" value="">
   										   				<input id="latitude_second_input" class="form-control" placeholder="42.36" name="latitude_second" type="number" value="">
   										   				</br>			   			
   										   				<label>Longitude</label>
   										   				<div class="radio" align="left" >
   										   				<fieldset id="longitude_group">	
   										   					<label>
   										   					<input type="radio" name="longitude_group" id="longitude_east" value="E" checked>E
   										   					</label>
   										   					<label>
   										   					<input type="radio" name="longitude_group" id="latitude_west" value="W" >W
   										   					</label>
   										   					</br>
   										   				</fieldset>
   										   				</div>
   										   				<input id="longitude_degree_input" class="form-control" placeholder="0" name="longitude_degree" type="number" value="">
   										   				<input id="longitude_minute_input" class="form-control" placeholder="22" name="longitude_minute" type="number" value="">
   										   				<input id="longitude_second_input" class="form-control" placeholder="14.869" name="longitude_second" type="number" value="">
   										   				</br>
   										   			</div>
   										   		</td> 
   										   		<td  id="td_submit_position" align="right">
   										   			<button align="right" id="btn_submit_position" class="btn btn-primary">Submit<span class="fa fa-arrow-right"></span></button>
   										   		</td>
   										   </tr>
   										   
    										   <tr>
   										   		<td>wappkey</td>
   										   		<td id="wappkey_value"></td>
												<td align="right">
												<input type="checkbox" id="wappkey_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if($gw_conf['wappkey']) echo "checked";?>>
												</td>
												<!-- <td align="right" ><button id="btn_edit_wappkey" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td> -->
   										   		
   										   		<td id="td_edit_wappkey">
   										   			<div id="div_wappkey" class="form-group">
   										   				<div class="radio">
   										   				<fieldset id="wappkey_group" >	
   										   				<label>
   										   					<input type="radio" name="wappkey_group" id="wappkey_true" value="true" <?php if($gw_conf['wappkey']) echo "checked";?> >True
   										   				</label>
   										   				</br>
   										   				<label>
   										   					<input type="radio" name="wappkey_group" id="wappkey_false" value="false" <?php if(!$gw_conf['wappkey']) echo "checked";?> >False
   										   				</label>
   										   				</fieldset>
   										   				</div>
   										   			</div>
   										   		</td>
   										   		<td id="td_wappkey_submit" align="right">
   										   			<button id="btn_wappkey_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										   		</td>
   										   </tr>
   										        										   
   										   <tr>
   										   		<td><a href="#" class="my_tooltip" data-toggle="tooltip" title="raw format disables the 4-byte packet header. Set to true for LoRaWAN.">raw format</a></td>
   										   		<td id="raw_value">
    								    		<?php
    								    			if ($radio_conf['mode']==11) {
    								    				if ($gw_conf['raw']==false)
    								    					echo '<font color="red">For LoRaWAN mode, set to true</font>';	
    								    			}
    								    		?>   										   		
   										   		</td>   										   		
												<td align="right">
												<input type="checkbox" id="raw_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if($gw_conf['raw']) echo "checked";?>>
												</td>
												<!-- <td align="right" ><button id="btn_edit_raw" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td> -->
												
   										   		<td id="td_edit_raw">
   										   			<div id="div_edit_raw" class="form-group">
   										   				<div class="radio">
   										   				<fieldset id="raw_group" >	
   										   				<label>
   										   					<input type="radio" name="raw_group" id="raw_true" value="true" <?php if($gw_conf['raw']) echo "checked";?> >True
   										   				</label>
   										   				</br>
   										   				<label>
   										   					<input type="radio" name="raw_group" id="raw_false" value="false" <?php if(!$gw_conf['raw']) echo "checked";?> >False
   										   				</label>
   										   				</fieldset>
   										   				</div>
   										   			</div>
   										   		</td>
   										   		<td id="td_raw_submit" align="right">
   										   			<button id="btn_raw_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										   		</td>
   										   </tr>

   										   <tr>
    									    <td><a href="#" class="my_tooltip" data-toggle="tooltip" title="aes_lorawan enables local data decryption. decryption keys must be stored in key_LoRaWAN.py. For LoRaWAN mode set to false to upload to network server.">aes_lorawan</a></td>
    										<td id="aes_lorawan_value">
    								    		<?php
    								    			if ($gw_conf['aes_lorawan']==true) {
    								    				if ($radio_conf['mode']==11)
    								    					echo '<font color="orange">For LoRaWAN mode set to false to upload to network server</font>';	
    								    			}
    								    			else
    								    				echo "for local decrypt";
    								    		?>     										
    										</td>
											<td align="right">
											<input type="checkbox" id="aes_lorawan_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if($gw_conf['aes_lorawan']) echo "checked";?>>
											</td>
											<!-- <td align="right"><button id="btn_edit_aes_lorawan" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td> -->
												
   										   	<td id="td_edit_aes_lorawan">
   										    	<div id="div_edit_aes_lorawan" class="form-group">
                                            		<label></label>
                                           			<div class="radio">
                                           			<fieldset id="aes_lorawan_group" >
                                                		<label>
                                                    		<input type="radio" name="aes_lorawan_group" id="aes_lorawan_true" value="true" <?php if($gw_conf['aes_lorawan']) echo "checked";?> >True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="aes_lorawan_group" id="aes_lorawan_false" value="false" <?php if(!$gw_conf['aes_lorawan']) echo "checked";?> >False
                                                		</label>
                                                		</fieldset>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_aes_lorawan_submit" align="right">
   										    		<button id="btn_aes_lorawan_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										      										   
   										   <tr>
    									    <td>aes</td>
    										<td id="aes_value">for local decrypt</td>
											<td align="right">
											<input type="checkbox" id="aes_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if($gw_conf['aes']) echo "checked";?>>
											</td>
											<!-- <td align="right"><button id="btn_edit_aes" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td> -->
											
   										   	<td id="td_edit_aes">
   										    	<div id="div_edit_aes" class="form-group">
                                            		<label></label>
                                           			<div class="radio">
                                           			<fieldset id="aes_group" >
                                                		<label>
                                                    		<input type="radio" name="aes_group" id="aes_true" value="true" <?php if($gw_conf['aes']) echo "checked";?> >True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="aes_group" id="aes_false" value="false" <?php if(!$gw_conf['aes']) echo "checked";?> >False
                                                		</label>
                                                		</fieldset>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_aes_submit" align="right">
   										    		<button id="btn_aes_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>

   										   <tr>
    									    <td>lsc</td>
    										<td id="lsc_value">for local decrypt</td>
											<td align="right">
											<input type="checkbox" id="lsc_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if($gw_conf['lsc']) echo "checked";?>>
											</td>
											<!-- <td align="right"><button id="btn_edit_lsc" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td> -->
											
   										   	<td id="td_edit_lsc">
   										    	<div id="div_edit_lsc" class="form-group">
                                            		<label></label>
                                           			<div class="radio">
                                           			<fieldset id="lsc_group" >
                                                		<label>
                                                    		<input type="radio" name="lsc_group" id="lsc_true" value="true" <?php if($gw_conf['lsc']) echo "checked";?> >True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="aes_group" id="aes_false" value="false" <?php if(!$gw_conf['lsc']) echo "checked";?> >False
                                                		</label>
                                                		</fieldset>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_lsc_submit" align="right">
   										    		<button id="btn_lsc_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										      										   
   										   <tr>
    									    <td><a href="#" class="my_tooltip" data-toggle="tooltip" title="For non-LoRaWAN downlink, specifying a value greater than 0 triggers the downlink checking process at both post-processing and lora_gateway level. Should be set to 0 for LoRaWAN downlink. Use -1 to disable downlink at gateway.">downlink</a></td>
    										<td id="downlink_value">
    											<?php 
    												echo $gw_conf['downlink'];
    								    			if ($radio_conf['mode']==11) {
    								    				if ($gw_conf['downlink']!=0)
    								    					echo '<font color="red"> [For LoRaWAN mode, set to 0]</font>';	
    								    			}										
    											?>
    										</td>
    										<td align="right"><button id="btn_edit_downlink" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_downlink">
   										    	<div id="div_update_downlink" class="form-group">
                                            		<label>Downlink timer</label>
                                					<input id="downlink_input" class="form-control" placeholder="downlink timer in seconds" name="downlink" type="number" value="" autofocus>
                                        			<p><font color="red">Enter 00 for 0. Specifying a value different from 0 triggers the downlink checking process at both post-processing and lora_gateway level. Run Gateway Update/Basic config to recompile lora_gateway with downlink support.</font></p>
                                        		</div>
                                        	</td> 
   										    <td id="td_downlink_submit" align="right">
   										    		<button id="btn_downlink_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr> 

    									    <td><a href="#" class="my_tooltip" data-toggle="tooltip" title="Specifying a value different from 0 will periodically trigger the post-processing_status script that handles periodic tasks.">status</a></td>
    										<td id="status_value">
    											<?php 
    												echo $gw_conf['status'];
    								    			if ($radio_conf['mode']==11) {
    								    				if ($gw_conf['status']==0)
    								    					echo '<font color="red"> [For LoRaWAN mode, set to 600 for instance]</font>';	
    								    			}										
    											?>
    										</td>
    										<td align="right"><button id="btn_edit_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_status">
   										    	<div id="div_update_status" class="form-group">
                                            		<label>Status timer</label>
                                					<input id="status_input" class="form-control" placeholder="status timer in seconds" name="status" type="number" value="" autofocus>
                                        			<p><font color="red">Enter 00 for 0. Specifying a value different from 0 will periodically trigger the post-processing_status script. Note that gateway stats report on LoRaWAN Network Server (e.g. TTN) will need a status value different from 0, e.g. 30s.</font></p>
                                        		</div>
                                        	</td> 
   										    <td id="td_status_submit" align="right">
   										    		<button id="btn_status_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr> 
   										      										       										   							   										   
										 </tbody>
    								    </table>
    								    <p>For LoRaWAN, if the gateway ID is 0000B827EBEFC4A6, then use B827EB<b>FFFF</b>EFC4A6 to register the gateway EUI on LoRaWAN network server platform such as TheThingsNetwork (TTN) for instance.</p>
    								    <p>For LoRaWAN mode, set raw format to true and aes_lorawan to false to upload the encrypted LoRaWAN packet to the Network Server.</p>    								    
    							      </div>
    							    </div>
    							
                                </div>
                                
                                <!-- tab-pane -->
                                
                                <div class="tab-pane fade" id="netserv-pills">
                                    <div id="netserv_msg"></div>
                                    
                                    <?php
                                    	echo '&nbsp;&nbsp;&nbsp;&nbsp;<a href="#" class="my_tooltip" data-toggle="tooltip" title="ChirpStack is an open-source LoRaWAN network server that can be installed locally on the gateway. It includes an application server so you can use it to handle all your LoRaWAN messages">ChirpStack</a>';
                                    	if (is_file("/lib/systemd/system/chirpstack-network-server.service")) {
                                    		echo " seems to be installed, OK.";
                                    	}
                                    	else {
                                    		echo " is not installed. Install it before enabling ChirpStack Network Server.";
                                    	}
                                    ?>	
                                    </br>
                                    </br>
                                 	<div class="col-md-10 col-md-offset-0">
                                    
                                    <div class="table-responsive">
										<table class="table table-striped table-bordered table-hover">
   										  <thead>
    								       
    									 </thead>
										<tbody>
   										   <tr>
											<?php														
												if (get_cloud_status($lorawan_encrypted_clouds, "python CloudChirpStack.py"))
													$cloud_chirpstack_enabled=true;
												else
													$cloud_chirpstack_enabled=false;	
											?>   										   	
    									    <td>Enabled</td>
    										<td id="chirpstack_status_value">
    										<?php
    											if ($chirpstack_not_active=='') {
    												echo '[<a href="/#/" onclick="javascript:event.target.port=8080">go to ChirpStack web page</a>]';	
    											}
    										?>
    										</td>
											<td align="right">
											<input type="checkbox" id="chirpstack_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if ($chirpstack_not_active=='') echo "checked";?>>											
											</td>
   										   </tr>
   										   
   										   <tr>
    									    <td><a href="#" class="my_tooltip" data-toggle="tooltip" title="All uplink messages will use CloudChirpStack.py. All downlink messages will use the Semtech's lora_pkt_fwd if you have a multi-channel gateway.">CloudChirpStack.py</a></td>
    										<td id="cloudchirpstack_status_value"></td>
											<td align="right">
											<input type="checkbox" id="cloudchirpstack_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if ($cloud_chirpstack_enabled) echo "checked";?>>										
											</td>   										   
   										   </tr>

											<tr>
												<td><a href="#" class="my_tooltip" data-toggle="tooltip" title="Use 127.0.0.1 for local ChirpStack server.">ChirpStack Server</a></td>
												<td id="td_cloudchirpstack_lorawan_server_value">
													<?php 
														echo $key_clouds['chirpstack_lorawan_server'];
													?>
												</td> 
												<td align="right">
													<button id="btn_edit_cloudchirpstack_lorawan_server" type="button" class="btn btn-primary">
														<span class="fa fa-edit" ></span>
													</button>
												</td>
												<td id="td_edit_cloudchirpstack_lorawan_server">
													<div id ="div_cloudchirpstack_lorawan_server" class="form-group">
														<label>ChirpStack server</label>
														<input id="cloudchirpstack_lorawan_server_input" class="form-control" placeholder="e.g. 127.0.0.1" type="text" value="" ></input>
													</div>
												</td>
												<td id="td_cloudchirpstack_lorawan_server_submit" align="right">
													<button id="btn_cloudchirpstack_lorawan_server_submit" type="submit" class="btn btn-primary">
													Submit
														<span class="fa fa-arrow-right"></span>
													</button>
												</td>
											</tr>
											
											<tr>
												<td><a href="#" class="my_tooltip" data-toggle="tooltip" title="Indicate a list of accepted device addresses, e.g. 6,7,0x01020304">source list</a></td>
												<td id="td_cloudchirpstack_source_list_value">
													<?php 
														$array = $key_clouds['chirpstack_source_list'];
														$size = count($key_clouds['chirpstack_source_list']);
														$i = 0;
														if($size == 0) echo "Empty";
														foreach( $array as $cel){
															if(($size-1) == $i){
																echo $cel;
															}else{
																echo $cel . ",";
															}
															$i++;
														}
													?>
												</td>
												<td align="right">
													<button id="btn_edit_cloudchirpstack_source_list" type="button" class="btn btn-primary">
														<span class="fa fa-edit"></span>
													</button>
												</td>
												<td id="td_edit_cloudchirpstack_source_list">
													<div id="div_edit_cloudchirpstack_source_list" class="form-group">
														<label>Add a sensor to your list</label>
														<input id="cloudchirpstack_source_list_input" class="form-control" placeholder="e.g. 6,7,0x01020304" type="text"
														value="<?php 
															$array = $key_clouds['chirpstack_source_list'];
															$size = count($key_clouds['chirpstack_source_list']);
															$i = 0;
															foreach( $array as $cel){
																if(($size-1) == $i){
																	echo $cel;
																}else{
																	echo $cel . ",";
																}
																$i++;
															}
														?>" autofocus>
													</div>
												</td>
												<td id="td_cloudchirpstack_source_list_submit" align="right">
													<button id="btn_cloudchirpstack_source_list_submit" type="submit" class="btn btn-primary">
														Submit
														<span class="fa fa-arrow-right"></span>
													</button>
												</td>
											</tr>
				
										</tbody>
    								 </table>
    								 <?php
    								 	if ($chirpstack_not_active=='') {
    								 		if ($cloud_chirpstack_enabled) {
    								 			if ($is_sx1301=='') {
    								 				echo '<p><font color="green">You have a single-channel gateway. ChirpStack is active and CloudChirpStack.py is enabled.</font></p>';

    								 				if ($radio_conf['mode']==11)
    								 					echo '<p><font color="green">Radio mode is for LoRaWAN. You can have <a href="#" class="my_tooltip" data-toggle="tooltip" title="All uplink LoRaWAN messages will use CloudChirpStack.py. Join-request for OTAA must use LoRa uplink setting.">limited</a> LoRaWAN support.</font></p>';
    								 				else
    								 					echo '<p><font color="orange">Radio mode is not for LoRaWAN. You have <a href="#" class="my_tooltip" data-toggle="tooltip" title="All uplink LoRaWAN-like messages can be uploaded to a LoRaWAN network server. LoRaWAN downlink messages are not handled.">only uplink</a> LoRaWAN-like support.</font></p>';     								 				
    								 			}
    								 			else {
    								 				echo '<p><font color="green">You have a multi-channel gateway. ChirpStack is active and CloudChirpStack.py is enabled.</font></p>';
    								 				if ($radio_conf['mode']==11)
    								 					echo '<p><font color="green">Radio mode is for LoRaWAN. You can have <a href="#" class="my_tooltip" data-toggle="tooltip" title="All uplink messages will use CloudChirpStack.py. All downlink messages will be handled by Semtech lora_pkt_fwd">full</a> LoRaWAN support.</font></p>';
    								 				else
    								 					echo '<p><font color="orange">Radio mode is not for LoRaWAN. You have <a href="#" class="my_tooltip" data-toggle="tooltip" title="All uplink LoRaWAN-like messages can be uploaded to a LoRaWAN network server. LoRaWAN downlink messages are not handled.">only uplink</a> LoRaWAN-like support.</font></p>'; 
    								 			}
    								 		}
    								 		else {
    								 			echo '<p><font color="orange">Warning: ChirpStack is active but CloudChirpStack.py is disabled.</font></p>'; 
    								 		}
    								 	}		
    								 ?>
    							  </div>
                                
                                
                                </div>  
                               </div>
                               
                                 <!-- tab-pane -->                                
                                
                                <div class="tab-pane fade" id="alert_mail-pills">
                                    </br>
                                    <div id="alert_mail_msg"></div>
                                 <div class="col-md-10 col-md-offset-0">
                                    
                                    <div class="table-responsive">
										<table class="table table-striped table-bordered table-hover">
   										  <thead>
    								       
    									 </thead>
										<tbody>
										   <tr>
    									    <td>Enabled</td>
    										<td id="use_mail_value"></td>									
											<td align="right">
											<input type="checkbox" id="use_mail_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if($alert_conf['use_mail']) echo "checked";?>>											
											</td>
											<!-- <td align="right"><button id="btn_edit_use_mail" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td> -->
											
   										   	<td id="td_edit_use_mail">
   										    	<div id="div_use_mail_options" class="form-group">
                                            		<label></label>
                                           			<div class="radio">
                                           			<fieldset id="use_mail_group" >
                                                		<label>
                                                    		<input type="radio" name="use_mail_group" id="use_mail_true" value="true" <?php if($alert_conf['use_mail']) echo "checked";?> >True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="use_mail_group" id="use_mail_false" value="false" <?php if(!$alert_conf['use_mail']) echo "checked";?> >False
                                                		</label>
                                                		</fieldset>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_use_mail_submit" align="right">
   										    		<button id="btn_use_mail_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										   
   										   <tr>
    									    <td>Mail Account</td>
    										<td id="mail_from_value"><?php  echo $alert_conf['mail_from']; ?></td>
    										<td align="right"><button id="btn_edit_mail_from" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_mail_from">
   										    	<div id="div_update_mail_from" class="form-group">
                                            		<label>Mail Account</label>
                                					<input id="mail_from_input" class="form-control" placeholder="mail account" name="mail_from" type="email" value="" autofocus>
                                        		</div>
                                        	</td> 
   										    <td id="td_mail_from_submit" align="right">
   										    		<button id="btn_mail_from_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										   
   										   <tr>
    									    <td>Mail Password</td>
    										<td id="mail_passwd_value"><?php echo md5($alert_conf['mail_passwd']); ?></td>
    										<td align="right"><button id="btn_edit_mail_passwd" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_mail_passwd">
   										    	<div id="div_update_mail_passwd" class="form-group">
                                            		<label>Mail Password</label>
                                					<input id="mail_passwd_input" class="form-control" placeholder="mail passwd" name="mail_passwd" type="text" value="" autofocus>
                                        		</div>
                                        	</td> 
   										    <td id="td_mail_passwd_submit" align="right">
   										    		<button id="btn_mail_passwd_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										   
   										   <tr>
    									    <td>Mail Server</td>
    										<td id="mail_server_value"><?php echo $alert_conf['mail_server']; ?></td>
    										<td align="right"><button id="btn_edit_mail_server" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_mail_server">
   										    	<div id="div_update_mail_server" class="form-group">
                                            		<label>Mail Server</label>
                                					<input id="mail_server_input" class="form-control" placeholder="smtp.gmail.com for example" name="mail_server" type="email" value="" autofocus>
                                        		</div>
                                        	</td> 
   										    <td id="td_mail_server_submit" align="right">
   										    		<button id="btn_mail_server_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										   
   										   <tr>
    									    <td>Contacts</td>
    										<td id="contact_mail_value"><?php echo $alert_conf['contact_mail']; ?></td>
    										<td align="right"><button id="btn_edit_contact_mail" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_contact_mail">
   										    	<div id="div_update_contact_mail" class="form-group">
                                            		<label>Contacts</label>
                                					<input id="contact_mail_input" class="form-control" placeholder="mail_1,mail_2,mail_3" name="contact_mail" type="text" value="<?php echo $alert_conf['contact_mail']; ?>" autofocus>
                                        			<p><font color="green">Mail addresses must be separated by commas, and without spaces.</font></p>
                                        		</div>
                                        	</td> 
   										    <td id="td_contact_mail_submit" align="right">
   										    		<button id="btn_contact_mail_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										   
										</tbody>
    								 </table>
    							  </div>
                                
                                </div>  
                      		
                            </div>  
                             
                            <!-- tab-pane -->
                                
                                <div class="tab-pane fade" id="alert_sms-pills">
                                    </br>
                                    <div id="alert_sms_msg"></div>
                                 <div class="col-md-10 col-md-offset-0">
                                    
                                    <div class="table-responsive">
										<table class="table table-striped table-bordered table-hover">
   										  <thead>
    								       
    									 </thead>
										<tbody>
   										   <tr>
    									    <td>Enabled</td>
    										<td id="use_sms_value"></td>
											<td align="right">
											<input type="checkbox" id="use_sms_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if($alert_conf['use_sms']) echo "checked";?>>											
											</td>
											<!-- <td align="right"><button id="btn_edit_use_sms" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td> -->
   										   	<td id="td_edit_use_sms">
   										    	<div id="div_use_sms_options" class="form-group">
                                            		<label></label>
                                           			<div class="radio">
                                           			<fieldset id="use_sms_group" >
                                                		<label>
                                                    		<input type="radio" name="use_sms_group" id="use_sms_true" value="true" <?php if($alert_conf['use_sms']) echo "checked";?> >True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="use_sms_group" id="use_sms_false" value="false" <?php if(!$alert_conf['use_sms']) echo "checked";?> >False
                                                		</label>
                                                		</fieldset>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_use_sms_submit" align="right">
   										    		<button id="btn_use_sms_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										   
   										   <tr>
    									    <td>Pin code</td>
    										<td id="sms_pin_value"><?php echo $alert_conf['pin']; ?></td>
    										<td align="right"><button id="btn_edit_sms_pin" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_sms_pin">
   										    	<div id="div_update_sms_pin" class="form-group">
                                            		<label>Pin code</label>
                                					<input id="sms_pin_input" class="form-control" placeholder="0000" name="sms_pin" type="text" pattern="[0-9]{4}" value="" autofocus>
                                        			<p><font color="red">Be sure that you can access to the sim card and thus change the pin code using telephone/smartphone.</font></p>
                                        		</div>
                                        	</td> 
   										    <td id="td_sms_pin_submit" align="right">
   										    		<button id="btn_sms_pin_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										   
   										   <tr>
    									    <td>Contacts</td>
    										<td id="contact_sms_value"><?php  display_array($alert_conf['contact_sms']); ?></td>
    										<td align="right"><button id="btn_edit_contact_sms" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_contact_sms">
   										    	<div id="div_update_contact_sms" class="form-group">
                                            		<label>Contacts</label>
                                					<input id="contact_sms_input" class="form-control" placeholder="+number_1,+number_2,+number_3" name="contact_sms" type="text" value="<?php  display_array($alert_conf['contact_sms']); ?>" autofocus>
                                        			<p><font color="green">Phone numbers must be separated by commas, and without spaces.</font></p>
                                        		</div>
                                        	</td> 
   										    <td id="td_contact_sms_submit" align="right">
   										    		<button id="btn_contact_sms_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
										</tbody>
    								 </table>
    							  </div>
                                
                                </div>  
                               </div>
                                 <!-- tab-pane -->
                                 
                                <div class="tab-pane fade" id="downlink-pills">
                                    </br>
                                    <div id="downlink_form_msg"></div>
                                    
                                    <div class="col-md-8 col-md-offset-1"> 
                    						<div class="panel-body">
                        						<form id="downlink_form" role="form">
                            						<fieldset>
                                						<div class="form-group">
                                							<label>Destination</label>
                                							<input class="form-control" placeholder="Between 2 and <?php echo $maxAddr; ?>" name="destination" type="number" value="" min="2" max="<?php echo $maxAddr; ?>" autofocus>
                                						</div>
                                						<div class="form-group">
                                							<label>Message</label>
                                							<input class="form-control" placeholder="message" name="message" type="text" value="" autofocus>
                                						</div>
                                						
                                						<center>
                                							<button  type="submit" class="btn btn-primary">Submit</button>
                                							<button  id="btn_downlink_form_reset" type="reset" class="btn btn-primary">Clear</button>
                                						</center> 
                            						</fieldset>
                        						</form>
                    						</div>
            						</div>	
                                    
                            </div>
                            
                            
                        		<div class="tab-pane fade" id="copy-log-pills">
                                   	</br></br>
                                   
                                   		<p>
                                   			<button id="btn_copy_log_file" type="button" class="btn btn-primary" href="process.php?copy_log_file=true">   <span class="fa fa-upload"></button>
                                   			Copy the current post-processing.log file, extract last 500 lines in a separate file and make links below available (right click to download).
                                   		</p>

										<table class="table table-striped table-bordered table-hover">
   										  <thead>
    								       
    									 </thead>
										<tbody>
										   <tr>
    									    <td>The entire content of post-processing.log</td>
    									    <td><a href="../log/post-processing.log">click here</a></td>
   										   </tr>
   					
   										   <tr>
    									    <td>Last 500 lines of post-processing.log</td>
    									    <td><a href="../log/post-processing-500L.log">click here</a></td>
   										   </tr>
										</tbody>
    								  </table>
                                   	
            						<div id="copy_log_file_msg"></div>	
                                	
                        		</div>                            
                                
                        <!-- /.panel-body -->
                        
                </div>
                <!-- /.panel -->
        
<?php require 'footer.php'; ?>
