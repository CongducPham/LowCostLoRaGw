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
                                <li><a href="#apmode-pills" data-toggle="tab">Switch to AP mode</a>
                                </li>
                                <li><a href="#cellular-pills" data-toggle="tab">Cellular</a>
                                </li>                                
                                <li><a href="../../raspap-webgui/">RaspAP webgui</a>
                                </li>                                                                                                 
								<li><a href="#profile-pills" data-toggle="tab">Web admin login settings</a>
         						</li>
                            </ul>
                            
                            </br>
            				<p>
								<?php 
									$hostapd=exec("pgrep hostapd");
									$dhcpcd_nohook=exec("grep 'nohook wpa_supplicant' /etc/dhcpcd.conf");
																	
									echo '<p>';
									if ($dhcpcd_nohook=='') {								
										echo "&nbsp;&nbsp;&nbsp;&nbsp;Gateway is normally configured as WiFi client. Connected to: ";
										$connected_wifi=exec("iwgetid | cut -d ':' -f2");
										if ($connected_wifi=='')
											echo "N/A";
										else	
											echo $connected_wifi;
										echo ". Use RaspAP to manage WiFi networks.";	
									}
									else{
										echo "&nbsp;&nbsp;&nbsp;&nbsp;Gateway is normally acting as WiFi Access Point.";
										if ($hostapd=='')
											echo '<font color="red"> hostapd not active, switch to AP mode again and reboot.</font>';
									}
									echo '</p>';
								?>                            
                            </p>
                                                        

                            <div class="tab-content">

                            <!-- Tab panes -->				
                            	<div class="tab-pane fade in active" id="hostapd-pills">
                                   </br>
                                    <div class="col-md-8 col-md-offset-1"> 
                    						<div class="panel-body">
                        						<form id="hostapd_form" role="form">
                            						<fieldset>
                                						<div class="form-group">
                                							<label>SSID</label>
                                							<input id="ssid" class="form-control" placeholder="current SSID is <?php $ssid=exec('egrep "^ssid=" /etc/hostapd/hostapd.conf | cut -d "=" --field=2'); echo $ssid; ?>" name="ssid" type="text" value="">	
                                							<p><font color="grey">default SSID would be WAZIUP_PI_GW_<?php $gw_id=exec('cat /home/pi/lora_gateway/gateway_id.txt | cut -c 5-'); echo $gw_id; ?></font></p>
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

                            <!-- Tab panes -->                                
                            	<div class="tab-pane fade" id="wificlient-pills">
                                   </br>
                                    <div class="col-md-8 col-md-offset-1"> 
                    						<div class="panel-body">
                        						<form id="wificlient_form" role="form">
                            						<fieldset>
                                						<div class="form-group">
                                							<label>SSID</label>
                                							<input id="wificlient_ssid" class="form-control" placeholder="your_wifi_network" name="wificlient_ssid" type="text" value="" autofocus>
                                							<p><font color="red">Warning: if a valid WiFi network is not configured you will not be able to connect through the gateway's access point anymore. If that happens, use wired Ethernet to switch back to access point mode. After submitting new WiFi, either reboot or switch to WiFi client now.</font></p>
                                						</div>
                                						<div class="form-group">
                                							<label>WPA Passphrase</label>
                                    						<input id="wificlient_wpa_phrasepass" class="form-control" placeholder="your_wifi_network_password" name="wificlient_wpa_phrasepass" type="password" value="">
                                						</div>

                                						<center>
                                							<button  type="submit" class="btn btn-primary">Submit new WiFi</button>
                                							<button  id="btn_wificlientnow" type="button" class="btn btn-primary" href="process.php?wificlientnow=true">Switch to WiFi client - now</button>
                                							<button  id="btn_wificlient_form_reset" type="reset" class="btn btn-primary">Clear</button>
                                						</center> 
                            						</fieldset>
                        						</form>
                    						</div>
            						</div>	
            						</br>
            						<div id="wificlient_msg"></div>	
                                </div>                                

                            <!-- Tab panes -->                           
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
                            	<div class="tab-pane fade" id="cellular-pills">
                                   </br>
                                   
                                	<p>
										<?php 
										
											//echo "&nbsp;&nbsp;&nbsp;&nbsp;Current cellular configuration: ";
											
											$dongle_on_boot=false;
											$loranga_on_boot=false;
											$loranga_3G=false;
											
											if(is_file("/home/pi/lora_gateway/3GDongle/use_3GDongle_internet_on_boot.txt")){
												//echo "Use 3G dongle for Internet on boot";
												$dongle_on_boot=true;
											}
											elseif(is_file("/home/pi/lora_gateway/3GDongle/loranga/use_loranga_internet_on_boot.txt")){
												//echo "Use Loranga for Internet on boot";
												$loranga_on_boot=true;												
											}
											
											if(is_file("/home/pi/lora_gateway/3GDongle/loranga/loranga3G.txt")){
												//echo " – With Loranga 3G";
												$loranga_3G=true;
											}																				
										?>  
                            		</p>
                            		
            						<div class="col-md-5 col-md-offset-0">
                                      <div class="table-responsive">
										<table class="table table-striped table-bordered table-hover">
   										  <thead></thead>
										 <tbody>
										   <tr>
    									    <td>Dongle on boot</td>
											<td align="right">
											<input type="checkbox" id="dongle_on_boot_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if($dongle_on_boot) echo "checked";?>>
											</td>    									    
   										   </tr>
   										   
   										   <tr>
    									    <td>Loranga on boot</td>
											<td align="right">
											<input type="checkbox" id="loranga_on_boot_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if($loranga_on_boot) echo "checked";?>>
											</td>    									    
   										   </tr>   										   
   										   
   										   <tr>
    									    <td>Loranga 3G/2G</td>
											<td align="right">
											<input type="checkbox" id="loranga_3G_2G_status_toggle" data-toggle="toggle" data-on="3G" data-off="2G" data-onstyle="success" data-offstyle="warning" <?php if($loranga_3G) echo "checked";?>>
											</td>    									    
   										   </tr>
									      										      										   
										 </tbody>
    								    </table>
    							      </div>
    							    </div>                            		
            						<div id="cellular_msg"></div>	
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
