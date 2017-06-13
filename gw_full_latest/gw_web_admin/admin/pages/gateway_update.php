<?php
include_once '../libs/php/functions.php';

require 'header.php';
?>


            <div class="navbar-default sidebar" role="navigation">
                <div class="sidebar-nav navbar-collapse">
                    <ul class="nav" id="side-menu">
                        </br>
                        
                        <li>
                            <a href="cloud.php"><i class="fa fa-dashboard fa-fw"></i> Cloud</a>
                        </li>
                        <li>
                            <a href="gateway_config.php"><i class="fa fa-dashboard fa-fw"></i> Gateway Configuration</a>
                        </li>
                        <li>
                            <a href="system.php"><i class="fa fa-dashboard fa-fw"></i> System</a>
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
                           
                        </div>
                         
                        <div class="panel-body">
                            <!-- Nav tabs -->
                            <ul class="nav nav-pills">
                                <li class="active"><a href="#new_install-pills" data-toggle="tab">New installation</a>
                                </li>
                                <li><a href="#full_update-pills" data-toggle="tab">Full update</a>
                                </li>
                                <li><a href="#update-pills" data-toggle="tab">Download and install a file</a>
                                </li>
                            </ul>

                            <!-- Tab panes -->
                            <div class="tab-content">
                            
                            	<div class="tab-pane fade in active" id="new_install-pills">
                                   	</br></br>
                                   
                                   		<p>
                                   			<button id="btn_gw_new_install" type="button" class="btn btn-primary" href="process.php?gw_new_install=true">   <span class="fa fa-arrow-right"></button>
                                   			Installing latest version of gateway.
                                   		</p>
                                   	
            						<div id="gw_new_install_msg"></div>	
                                	
                        		</div>
                        		<!-- tab-pane -->
                        		
                        		<div class="tab-pane fade" id="full_update-pills">
                                   	</br></br>
                                   
                                   		<p>
                                   			<button id="btn_gw_full_update" type="button" class="btn btn-primary" href="process.php?gw_full_update=true">   <span class="fa fa-arrow-right"></button>
                                   			Updating to latest version of gateway, preserving your local configuration files.
                                   		</p>
                                   	
            						<div id="gw_full_update_msg"></div>	
                                	
                        		</div>
                        		<!-- tab-pane -->
                        		
                            	<div class="tab-pane fade" id="update-pills">
                                   </br>
                                    <div class="col-md-8 col-md-offset-1"> 
                    						<div class="panel-body">
                        						<form id="gw_update_form" role="form">
                            						<fieldset>
                                						<div class="form-group">
                                							<label>URL of the file</label>
                                							<input class="form-control" placeholder="" name="file_name_url" type="text" value="" autofocus>	
                                						</div>
                                					
                                						<center>
                                							<button  type="submit" class="btn btn-primary">Install</button>
                                							<button  id="btn_gw_update_form_reset" type="reset" class="btn btn-primary">Clear</button>
                                						</center> 
                            						</fieldset>
                        						</form>
                    						</div>
            						</div>	
            						</br>
            						<div id="gw_update_form_msg"></div>	
                                </div> 
                                <!-- tab-pane -->
                        </div>
                        <!-- /.panel-body -->
                </div>
                <!-- /.panel -->
    
    

<?php require 'footer.php'; ?>
