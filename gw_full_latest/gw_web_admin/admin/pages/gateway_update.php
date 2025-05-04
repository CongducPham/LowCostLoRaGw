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
                    <h1 class="page-header">Gateway Update</h1>
                </div>
            </div>
            <!-- /.row -->
          
            <div class="panel panel-default">
                        <div class="panel-heading">
                          	<div id="gw_update_msg"></div>
                        </div>
                         
                        <div class="panel-body">
                            <!-- Nav tabs -->
                            <ul class="nav nav-pills">
                                <li class="active"><a href="#new_install-pills" data-toggle="tab">New installation</a>
                                </li>
                                <li><a href="#full_update-pills" data-toggle="tab">Full update</a>
                                </li>
                                <li><a href="#basic_conf-pills" data-toggle="tab">Basic config</a>
                                </li>
                                <li><a href="#update-pills" data-toggle="tab">Download and install a file</a>
                                </li>
				 				<li><a href="#update_web_admin-pills" data-toggle="tab">Update web admin interface</a>
                                </li>
                            </ul>

							</br>
            				<p>&nbsp;&nbsp;&nbsp;&nbsp;Run <b>Basic config</b> after any update and reboot for new version to be applied.</p>
            				
                            <!-- Tab panes -->
                            <div class="tab-content">
                            
                            	<div class="tab-pane fade in active" id="new_install-pills">
                                   	</br>
                                   
                                   		<table border="0">
                                   		<tr>
                                   			<td rowspan="2">
                                   				<button id="btn_gw_new_install" type="button" class="btn btn-primary" href="process.php?gw_new_install=true">   <span class="fa fa-upload"></button>
                                   			</td>
                                   			<td>                                   				
                                   				&nbsp;&nbsp;Install latest version of gateway, <strong>erasing</strong> all existing configuration file. 
                                   			</td>	
                                   		</tr>
                                   		<tr>
                                   			<td>	
                                   				&nbsp;&nbsp;Custom SSID will be preserved. May take minutes, wait for finish notification.
                                   			</td>
                                   		</tr>		
                                   		</table>

										</br>
										
										<?php
											$current_install_date=exec('stat --format=%y /home/pi/lora_gateway/post_processing_gw.py');
											if (is_file('/home/pi/lora_gateway/VERSION.txt')) {
												$installed_version=exec('cat /home/pi/lora_gateway/VERSION.txt');
											}
											else {	
												$installed_version=exec('cat /home/pi/VERSION.txt');
											}
											$git_version=exec('cat /home/pi/git-VERSION.txt');

											echo "Git version: $git_version. Installed version: $installed_version. Date of current distribution is $current_install_date";    
										?> 
						                                   	
            						<div id="gw_new_install_msg"></div>	
                                	
                        		</div>
                        		<!-- tab-pane -->
                        		
                        		<div class="tab-pane fade" id="full_update-pills">
                                   	</br>
                                   
                                   		<table border="0">
                                   		<tr>
                                   			<td rowspan="2">
                                   				<button id="btn_gw_full_update" type="button" class="btn btn-primary" href="process.php?gw_full_update=true"> <span class="fa fa-upload"></button>
                                   			</td>
                                   			<td>
                                   				&nbsp;&nbsp;Update to latest version of gateway, <strong>preserving</strong> your local configuration files.
                                   			</td>	
                                   		</tr>
                                   		<tr>
                                   			<td>	
                                   				&nbsp;&nbsp;Custom SSID will be preserved. May take minutes, wait for finish notification.
                                   			</td>
                                   		</tr>		
                                   		</table>

										</br>
										<?php
											$current_install_date=exec('stat --format=%y /home/pi/lora_gateway/post_processing_gw.py');
											if (is_file('/home/pi/lora_gateway/VERSION.txt')) {
												$installed_version=exec('cat /home/pi/lora_gateway/VERSION.txt');
											}
											else {	
												$installed_version=exec('cat /home/pi/VERSION.txt');
											}
											$git_version=exec('cat /home/pi/git-VERSION.txt');

											echo "Git version: $git_version. Installed version: $installed_version. Date of current distribution is $current_install_date";   
										?> 
										                                   	
            						<div id="gw_full_update_msg"></div>	
                                	
                        		</div>
                        		
                        		<!-- tab-pane -->
                        		
                        		<div class="tab-pane fade" id="basic_conf-pills">
                                   	</br>
                                   
                                   		<p>
                                   			<button id="btn_gw_basic_conf" type="button" class="btn btn-primary" href="process.php?gw_basic_conf=true">   <span class="fa fa-edit"></button>
                                   			Configure your gateway after new installation/update or after changing PA_BOOST/downlink settings.
                                   		</p>

										</br>
										<?php
											$current_install_date=exec('stat --format=%y /home/pi/lora_gateway/lora_gateway 2>/dev/null');
											
											if ($current_install_date=='') {
												echo "Date of last configuration undefined";
											}
											else {
												echo "Date of last basic configuration is $current_install_date";
											}
										?> 
										                                   	
            						<div id="gw_basic_conf_msg"></div>	
                                	
                        		</div>
                        		
                        		<!-- tab-pane -->
                        		
                            	<div class="tab-pane fade" id="update-pills">
                                   </br>
                                    <div class="col-md-8 col-md-offset-1"> 
                    						<div class="panel-body">
                        						<form id="gw_download_file_form" role="form">
                            						<fieldset>
                                						<div class="form-group">
                                							<label>URL of the file</label>
                                							<input class="form-control" placeholder="" name="file_name_url" type="text" value="" autofocus>	
                                						</div>
                                                        <p>Enter a URL link to a file or .zip archive. A .zip archive will be decompressed and its content will overwrite existing files. All files will be downloaded to <tt>/home/pi/lora_gateway</tt>.
                                                        <p>e.g. a Dropbox link like <tt>https://www.dropbox.com/s/usc9w3i6urbquvw/gateway_conf.json</tt></p>
                                                        
                                                    	<p>Erase all content of <tt>/home/pi/lora_gateway</tt>. Usually used with a .zip file to install a specific distribution. <strong>Use with extreme caution</strong>
														<input type="checkbox" name="gw_download_file_form_erase_checkbox" value="Yes" /> </p>
	                                    					
                                						<center>
                                							<button  type="submit" class="btn btn-primary">Install</button>
                                							<button  id="btn_gw_download_file_form_reset" type="reset" class="btn btn-primary">Clear</button>
                                						</center> 
                            						</fieldset>
                        						</form>
                    						</div>
            						</div>	
            						</br>
            						<div id="gw_download_file_form_msg"></div>	
                                </div> 
                                <!-- tab-pane -->

								<div class="tab-pane fade" id="update_web_admin-pills">
                                   	</br>
                    
                                   		<p>
                                   			<button id="btn_update_web_admin" type="button" class="btn btn-primary" href="process.php?update_web_admin=true">   <span class="fa fa-upload"></button>
                                   			Update to latest version of gateway web admin interface. May take 1 minute, wait for finish notification.
                                   		</p>    

										</br>
										<?php
											$current_install_date=exec('stat --format=%y /var/www/html/admin/index.php');

											echo "Date of installed web admin interface is $current_install_date";   
										?> 
										                                   		                         	
                        		</div>

                        </div>
                        <!-- /.panel-body -->
                </div>
                <!-- /.panel -->
    
    

<?php require 'footer.php'; ?>
