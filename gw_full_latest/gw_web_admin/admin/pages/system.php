<?php
include_once '../libs/php/functions.php';

// begin our session
session_start();

// check if the user is logged out
if(!isset($_SESSION['username'])){
                header('Location: login.php');
                exit();
}

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
                            <a href="gateway_config.php"><i class="fa fa-edit"></i> Gateway Configuration</a>
                        </li>
                        <li>
                            <a href="gateway_update.php"><i class="fa fa-upload"></i> Gateway Update</a>
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
                    <h1 class="page-header">System</h1>
                </div>
            </div>
            <!-- /.row -->
          
            <div class="panel panel-default">
                        <div class="panel-heading">
                           <div id="system_msg"></div>
                        </div>
                         
                        <div class="panel-body">
                            <!-- Nav tabs -->
                            <ul class="nav nav-pills">
                                <li class="active"><a href="#hostapd-pills" data-toggle="tab">GW Access Point</a>
                                </li>
                                <li><a href="#wificlient-pills" data-toggle="tab">Configure as WiFi client</a>
                                </li> 
                                <li><a href="#apmode-pills" data-toggle="tab">Switch back to AP mode</a>
                                </li>
                                <li><a href="../../raspap-webgui/">RaspAP webgui</a>
                                </li>                                                                                                 
								<li><a href="#profile-pills" data-toggle="tab">Web admin login settings</a>
         						</li>
                            </ul>
                            
                            </br>
            				<p>
								<?php 
									if(is_file("/etc/network/interfaces_not_ap")){
										echo "&nbsp;&nbsp;&nbsp;&nbsp;Gateway configuration file is for Access Point mode";
									}
									else{
										echo "&nbsp;&nbsp;&nbsp;&nbsp;Gateway configuration file is for WiFi client mode";
									}
								?>                            
                            </p>
                                                        
                            <!-- Tab panes -->
                            <div class="tab-content">
				
                            	<div class="tab-pane fade in active" id="hostapd-pills">
                                   </br>
                                    <div class="col-md-8 col-md-offset-1"> 
                    						<div class="panel-body">
                        						<form id="hostapd_form" role="form">
                            						<fieldset>
                                						<div class="form-group">
                                							<label>SSID</label>
                                							<input id="ssid" class="form-control" placeholder="current SSID is <?php $ssid=exec('egrep "^ssid=" /etc/hostapd/hostapd.conf | cut -d "=" --field=2'); echo $ssid; ?>" name="ssid" type="text" value="">	
                                							<p><font color="grey">default SSID would be WAZIUP_PI_GW_<?php $gw_id=exec('cat /home/pi/lora_gateway/gateway_id.txt | cut -c 7-'); echo $gw_id; ?></font></p>
                                						</div>
                                						<div class="form-group">
                                							<label>WPA Passphrase</label>
                                    						<input id="wpa_phrasepass" class="form-control" placeholder="wpa_passphrase" name="wpa_phrasepass" type="text" value="">
                                						</div>

                                						<center>
                                							<button  type="submit" class="btn btn-primary">Submit</button>
                                							<button  id="btn_hostapd_form_reset" type="reset" class="btn btn-primary">Clear</button>
                                						</center> 
                            						</fieldset>
                        						</form>
                    						</div>
            						</div>	
            						</br>
            						<div id="hostapd_msg"></div>	
                                </div> 
                                
                            	<div class="tab-pane fade" id="wificlient-pills">
                                   </br>
                                    <div class="col-md-8 col-md-offset-1"> 
                    						<div class="panel-body">
                        						<form id="wificlient_form" role="form">
                            						<fieldset>
                                						<div class="form-group">
                                							<label>SSID</label>
                                							<input id="wificlient_ssid" class="form-control" placeholder="your_wifi_network" name="wificlient_ssid" type="text" value="" autofocus>
                                							<p><font color="red">Warning: if a valid WiFi network is not configured you will not be able to connect through the gateway's access point anymore. If that happens, use wired Ethernet to switch back to access point mode. You MUST reboot after submitting the command.</font></p>
                                						</div>
                                						<div class="form-group">
                                							<label>WPA Passphrase</label>
                                    						<input id="wificlient_wpa_phrasepass" class="form-control" placeholder="your_wifi_network_password" name="wificlient_wpa_phrasepass" type="password" value="">
                                						</div>

                                						<center>
                                							<button  type="submit" class="btn btn-primary">Submit</button>
                                							<button  id="btn_wificlient_form_reset" type="reset" class="btn btn-primary">Clear</button>
                                						</center> 
                            						</fieldset>
                        						</form>
                    						</div>
            						</div>	
            						</br>
            						<div id="wificlient_msg"></div>	
                                </div>                                
                           
                            	<div class="tab-pane fade" id="apmode-pills">
                                   </br>
										<center>
											<p><font color="red">Warning: Internet connectivity should then be provided by wired Ethernet or 2G/3G</font></p>
											<p><font color="red">Warning: If using the "now" mode, you may loose current connection. Connect to gateway's access point WiFi.</font></p>
											<button  id="btn_apmode" type="button" class="btn btn-primary" href="process.php?apmode=true">Switch to AP mode - next reboot</button>
											<button  id="btn_apmodenow" type="button" class="btn btn-primary" href="process.php?apmodenow=true">Switch to AP mode - now</button>
										</center> 
            						</br>
            						<div id="apmode_msg"></div>	
                                </div> 
                                                            
                            <!-- Tab panes -->
                            	<div class="tab-pane fade" id="profile-pills">
                                   </br>
                                    <div class="col-md-8 col-md-offset-1"> 
                    						<div class="panel-body">
                        						<form id="profile_form" role="form">
                            						<fieldset>
                                						<div class="form-group">
                                							<label>Current Username</label>
                                							<input id="current_username" class="form-control" placeholder="username" name="current_username" type="text" value="" autofocus>
                                			                        	</div>

									<div class="form-group">
                                							<label>New Username</label>
                                							<input id="new_username" class="form-control" placeholder="username" name="new_username" type="text" value="" autofocus>
                                			                        	</div>

                                						<div class="form-group">
                                							<label>Current Password</label>
                                    						<input id="current_pwd" class="form-control" placeholder="Current password"  name="current_pwd" type="password" value="">
                                						</div>
									
									<div class="form-group">
                                							<label>New Password</label>
                                    						<input id="new_pwd" class="form-control" placeholder="New password" name="new_pwd" type="password" value="">
                                						</div>

                                						<center>
                                							<button  type="submit" class="btn btn-primary">Submit</button>
                                							<button  id="btn_profile_form_reset" type="reset" class="btn btn-primary">Clear</button>
                                						</center> 
                            						</fieldset>
                        						</form>
                    						</div>
            						</div>	
            						</br>
            						<div id="profile_msg"></div>	
                                </div> 
                            </div>

                        </div>
                        <!-- /.panel-body -->
                </div>
                <!-- /.panel -->
    
    

<?php require 'footer.php'; ?>
