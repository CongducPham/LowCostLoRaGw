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
                                <li><a href="#alert_mail-pills" data-toggle="tab">Alert Mail</a>
                                </li>
                                <li><a href="#alert_sms-pills" data-toggle="tab">Alert SMS</a>
                                </li>
                                <li><a href="#downlink-pills" data-toggle="tab">Downlink Request</a>
                                </li>
                                <li><a href="#copy-log-pills" data-toggle="tab">Get post-processing.log file</a>
                                </li>                                
                            </ul>

                            <!-- Tab panes -->
                            <div class="tab-content">
                            	<div class="tab-pane fade in active" id="radio_conf-pills">
                                    </br></br>
                                    
                                    <div id="radio_msg"></div>
                                    
                                    <div class="col-md-10 col-md-offset-0">
                                      <div class="table-responsive">
										<table class="table table-striped table-bordered table-hover">
   										  <thead>
    								       
    									 </thead>
										<tbody>
										   <tr>
    									    <td>Mode</td>
    										<td id="mode_value"><?php echo $radio_conf['mode']; ?></td>
    										<td align="right"><button id="btn_edit_mode" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										    <td id="td_edit_mode">
   										    	<div id="div_mode_select" class="form-group">
                                            		<label>Select a mode</label>
                                            		<select id="mode_select" class="form-control">
                                                		<option selected>1</option> <option>2</option> <option>3</option> <option>4</option>
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
    									    <td>Frequency</td>
    										<td id="freq_value"><?php echo $radio_conf['freq']; ?></td>
    										<td align="right"><button id="btn_edit_freq" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_freq">
   										    	<div class="form-group">
                                            		<label>ISM Band </label>
                                            		<select id="band_freq_select" class="form-control">
                                                		<option></option> <option>433MHz</option> <option>868MHz</option> <option>915MHz</option> 
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
    												$current_paboost=exec('egrep ^CFLAGS /home/pi/lora_gateway/radio.makefile');
    												
    												if ($current_paboost=='')
													{
    													echo "Disabled";    
													}
													else {
   					 									echo "Enabled";   
													}
    											?>    										
    										</td>
    										<td align="right"><button id="btn_edit_paboost" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_paboost">
   										    	<div id="div_paboost_options" class="form-group">
                                            		<label></label>
                                           			<div class="radio">
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="paboost_true" value="Enabled" checked>Enabled
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="paboost_false" value="Disabled" checked>Disabled
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
    								  <p>PA_BOOST is required for some radio modules such as inAir9B, RFM92W, RFM95W, NiceRF LoRa1276</p>
    								  <p>After changing the PA_BOOST settings, run <b>Gateway Update/Basic config</b> to recompile the low-level gateway program</p> 
    							     </div>
    							     
    							  </div>
    							                           		
                                </div>
                                <!-- tab-pane -->
                                
                                <div class="tab-pane fade" id="gw_conf-pills">
                                    </br>
                                    <!-- <h4>Gateway settings</h4></br> -->
                                    <div id="gw_msg"></div>
                                    <!-- Tab panes -->
                
                                    <div class="col-md-10 col-md-offset-0">
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
                                        		</div>
                                        	</td> 
   										    <td id="td_gw_id_submit" align="right">
   										    		<button id="gw_id_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   
   										   </tr>
   										   <tr>
    									    <td>AES</td>
    										<!-- <td id="aes_value"><?php echo $gw_conf['aes']; ?></td> -->
    										<td id="aes_value">
    											<?php
    												if($gw_conf['aes'])
													{
    													echo "true";    
													}
													else {
   					 									echo "false";   
													}
    											?>
    										</td>
    										<td align="right"><button id="btn_edit_aes" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_aes">
   										    	<div id="div_aes_options" class="form-group">
                                            		<label></label>
                                           			<div class="radio">
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="aes_true" value="true" checked>True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="aes_false" value="false" checked>False
                                                		</label>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_aes_submit" align="right">
   										    		<button id="btn_aes_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										   <tr>
    									    <td>downlink</td>
    										<td id="downlink_value">
    											<?php echo $gw_conf['downlink'];?>
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
   										   
										 </tbody>
    								    </table>
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
    										<!-- <td id="use_mail_value"><?php echo $alert_conf['use_mail']; ?></td> -->
    										<td id="use_mail_value">
    											<?php
    												if($alert_conf['use_mail'])
													{
    													echo "true";    
													}
													else {
   					 									echo "false";   
													}
    											?>
    										</td>
    										<td align="right"><button id="btn_edit_use_mail" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_use_mail">
   										    	<div id="div_use_mail_options" class="form-group">
                                            		<label></label>
                                           			<div class="radio">
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="use_mail_true" value="true" checked>True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="use_mail_false" value="false" >False
                                                		</label>
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
    										<!-- <td id="use_sms_value"><?php echo $alert_conf['use_sms']; ?></td> -->
    										<td id="use_sms_value">
    											<?php
    												if($alert_conf['use_sms'])
													{
    													echo "true";    
													}
													else {
   					 									echo "false";   
													}
    											?>
    										</td>
    										<td align="right"><button id="btn_edit_use_sms" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_use_sms">
   										    	<div id="div_use_sms_options" class="form-group">
                                            		<label></label>
                                           			<div class="radio">
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="use_sms_true" value="true" >True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="use_sms_false" value="false" checked>False
                                                		</label>
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
                                							<input class="form-control" placeholder="Between 2 and 255" name="destination" type="number" value="" min="2" max="255" autofocus>
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
