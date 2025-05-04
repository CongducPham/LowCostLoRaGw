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
<head>
<META HTTP-EQUIV="Refresh" CONTENT="5">
</head>

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
                    <h1 class="page-header">Packet logger</h1>
                </div>
            </div>
            <!-- /.row -->
          
            <div class="panel panel-default">
                        <div class="panel-heading">
                           <div id="pkt_logger_msg"></div>
                        </div>
                         
                        <div class="panel-body">
                            <!-- Nav tabs -->
                            <ul class="nav nav-pills">
                                <li class="active"><a href="#rxlora" data-toggle="tab">rxlora</a>
                                </li>
                                <!--
                                <li><a href="#cloud_script_calls" data-toggle="tab">Cloud script calls</a>
                                </li>
                                <li><a href="#ttn" data-toggle="tab">TTN</a>
                                </li>                                  
                                <li><a href="#errors" data-toggle="tab">Errors</a>
                                </li>
                                -->
                            </ul>
                            
                            </br>

                            <div class="tab-content">

                            <!-- Tab panes -->				
                            	<div class="tab-pane fade in active" id="rxlora">
                                   </br>
                                    <div class="col-md-12 col-md-offset-0"> 
                    						<div class="panel-body">
                    						<p>Showing last 20 packet receptions. Refreshing every 5s.</p>
                    						
											<p>
											<?php 
												ob_start(); 
												//system("tac /home/pi/lora_gateway/log/post-processing.log | egrep -a -m 20 'rxlora|: 256,.*,0x.*,'", $retval);
												system("tac /home/pi/lora_gateway/log/post-processing.log | egrep -a -m 20 '\+\+\+ rxlora'", $retval);
												$string=ob_get_contents(); 
												ob_clean();
												$patterns = array();
												$patterns[0] = '/(.+)SF=(\d+)/i';
												//$patterns[1] = '/update(.+)-(\d+)/i';
												$replacements = array(); 
												$replacements[0] = '${1}SF=$2<br>';
												//$replacements[1] = 'update${1}-$2<br>';
												echo preg_replace($patterns, $replacements, $string);				 
											?>					
											</p>
 
                    						</div>
            						</div>	
            						</br>
            						<div id="rxlora_msg"></div>	
                                </div>

                            <!-- Tab panes                                
                            	<div class="tab-pane fade" id="cloud_script_calls">
                                   </br>
                                    <div class="col-md-8 col-md-offset-1"> 
                    						<div class="panel-body">


                    						</div>
            						</div>	
            						</br>
            						<div id="cloud_script_calls_msg"></div>	
                                </div>                                
							--> 
							
                            <!-- Tab panes                            
                            	<div class="tab-pane fade" id="ttn">
                                   </br>
                                    <div class="col-md-8 col-md-offset-1"> 
                    						<div class="panel-body">


                    						</div>
            						</div>	
            						</br>
            						<div id="cloud_script_calls_msg"></div>	
                                </div>  
							-->
							
                            <!-- Tab panes                           
                            	<div class="tab-pane fade" id="errors">
                                   </br>
                                    <div class="col-md-8 col-md-offset-1"> 
                    						<div class="panel-body">


                    						</div>
            						</div>	
            						</br>
            						<div id="errors_msg"></div>	
                                </div>
                            -->     
                                                                                            
                            </div>

                        </div>
                        <!-- /.panel-body -->
                </div>
                <!-- /.panel -->
    
    

<?php require 'footer.php'; ?>
