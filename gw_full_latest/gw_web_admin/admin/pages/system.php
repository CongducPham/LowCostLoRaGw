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
                                <li class="active"><a href="#hostapd-pills" data-toggle="tab">Access Point</a>
                                </li>
				<li><a href="#profile-pills" data-toggle="tab">Web admin login settings</a>
         			</li>
                            </ul>
                            
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
                                							<input id="ssid" class="form-control" placeholder="WAZIUP_PI_GW_XXXXXXXXXX" name="ssid" type="text" value="" autofocus>
                                							<p><font color="grey">where XXXXXXXXXX is the last 5 bytes of your MAC Ethernet interface address.</font></p>
                                						</div>
                                						<div class="form-group">
                                							<label>WPA Passphrase</label>
                                    						<input id="wpa_phrasepass" class="form-control" placeholder="wpa_passphrase" name="wpa_phrasepass" type="password" value="">
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
            						<div id="form_msg"></div>	
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
