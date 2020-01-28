<?php

?>

	<div id="waziup_status_msg"></div>
	
	<div class="col-md-10 col-md-offset-0">
		<?php
			$date=date('Y-m-d\TH:i:s');
			echo '<p>Date/Time: ';
			echo $date;
			echo '</p>';											
			ob_start();
			system("tac /home/pi/lora_gateway/log/post-processing.log | egrep -a -m 1 'uploading with python.*CloudWAZIUP.*py' | cut -d '>' -f1"); 
			//system("egrep -a 'uploading with python.*CloudWAZIUP.*py' /home/pi/lora_gateway/log/post-processing.log | tail -1 | cut -d '>' -f1");
			$last_upload=ob_get_contents(); 
			ob_clean();
			echo '<p>';
			if ($last_upload=='') {
				echo '<font color="red"><b>no upload with CloudWAZIUP.py found</b></font>';					
			}
			else {
				$date=str_replace("T", " ", $date, $count);
				$datetime1 = new DateTime($date);
				$last_upload_1=str_replace("T", " ", $last_upload, $count);
				$datetime2 = new DateTime($last_upload_1);
				$interval = $datetime1->diff($datetime2);			
				echo 'last upload time with CloudWAZIUP.py: <font color="green"><b>';
				echo $last_upload;
				echo $interval->format(' %mm-%dd-%hh-%imin from current date');	
				echo '</b></font>';					
			}
			echo '</p>';									
		?>

		<div class="table-responsive">
		<table class="table table-striped table-bordered table-hover">
			<thead></thead>
			<tbody>
			<tr>
			<td>Enabled
				<?php 
					ob_start(); 
					$server=$key_clouds['waziup_server'];
					system("wget -q --tries=3 --spider $server/devices", $retval);
					$msg=ob_get_contents(); 
					ob_clean();
					if ($retval == 0) {
						echo ' <font color="green"><b>[server online]</b></font>';					
					}
					else {
						echo ' <font color="red"><b>[server offline]</b></font>';					
					}				 
				?>
			</td>
			<td id="waziup_status_value">								
			</td>
			
			<td align="right">
			<input type="checkbox" id="waziup_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if(get_cloud_status($clouds, "python CloudWAZIUP.py")) echo "checked";?>>
			</td>
			
			<!-- <td align="right"><button id="btn_edit_waziup_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>  -->
			<td id="td_edit_waziup_status">
				<div id="div_waziup_status_options" class="form-group">
					
					<div class="radio">
					<fieldset id="waziup_status_group" >
						<label>
							<input type="radio" name="waziup_status_group" id="waziup_true" value="true" <?php if(get_cloud_status($clouds, "python CloudWAZIUP.py")) echo "checked";?>>True
						</label>
						</br>
						<label>
							<input type="radio" name="waziup_status_group" id="waziup_false" value="false" <?php if(get_cloud_status($clouds, "python CloudWAZIUP.py")) echo "checked";?>>False
						</label>
					</div>
				</div>
			</td> 
			<td id="td_waziup_status_submit" align="right">
					<button id="btn_waziup_status_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
			</td>
			</tr>
			
			<tr>   
			<td>project name</td>
			<td id="project_value"><?php echo $key_clouds['project_name']; ?></td>
			<td align="right"><button id="btn_edit_project" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
			<td id="td_edit_project">
				<div id="div_update_project" class="form-group">
					<label>Project name</label>
					<input id="project_input" class="form-control" placeholder="e.g. waziup" name="project_name" type="text" value="" autofocus>
				</div>
			</td> 
			<td id="td_project_submit" align="right">
					<button id="project_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
			</td>
			</tr>
			
			<tr>
			<td><a href="#" class="my_tooltip" data-toggle="tooltip" title="Use a short name with no space, e.g. UPPA">organization name</a></td>
			<td id="organization_value"><?php echo $key_clouds['organization_name']; ?></td>
			<td align="right"><button id="btn_edit_organization" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
			<td id="td_edit_organization">
				<div id="div_update_organization" class="form-group">
					<label>Organization name</label>
					<input id="organization_input" class="form-control" placeholder="e.g. UPPA" name="organization_name" type="text" value="" autofocus>
				</div>
			</td> 
			<td id="td_organization_submit" align="right">
					<button id="organization_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
			</td>
			</tr>
			
			<tr>
			<td><a href="#" class="my_tooltip" data-toggle="tooltip" title="Use a short service tree with no space. Must begin with '-', e.g. -OFFICE1-TESTS">service tree</a></td>
			<td id="service_tree_value"><?php echo $key_clouds['service_tree']; ?></td>
			<td align="right"><button id="btn_edit_service_tree" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
			<td id="td_edit_service_tree">
				<div id="div_update_service_tree" class="form-group">
					<label>Service tree, must begin with '-'</label>
					<input id="service_tree_input" class="form-control" placeholder="e.g. -OFFICE1-TESTS" name="service_tree" type="text" value="" autofocus></div>
			</td> 
			<td id="td_service_tree_submit" align="right">
					<button id="service_tree_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
			</td>
			</tr>

			<tr>
			<td><a href="#" class="my_tooltip" data-toggle="tooltip" title="Create it on WAZIUP dashboard, only letters and numbers.">username</a></td>
			<td id="username_value"><?php echo $key_clouds['username']; ?></td>
			<td align="right"><button id="btn_edit_username" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
			<td id="td_edit_username">
				<div id="div_update_username" class="form-group">
					<label>username</label>
					<input id="username_input" class="form-control" placeholder="username [A-Za-z0-9]" name="username" type="text" value="" autofocus></div>
			</td> 
			<td id="td_username_submit" align="right">
					<button id="username_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
			</td>
			</tr>

			<tr>
			<td><a href="#" class="my_tooltip" data-toggle="tooltip" title="Create it on WAZIUP dashboard, only letters and numbers.">password</a></td>
			<td id="password_value"><?php echo $key_clouds['password']; ?></td>
			<td align="right"><button id="btn_edit_password" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
			<td id="td_edit_password">
				<div id="div_update_password" class="form-group">
					<label>password</label>
					<input id="password_input" class="form-control" placeholder="password [A-Za-z0-9]" name="password" type="text" value="" autofocus></div>
			</td> 
			<td id="td_password_submit" align="right">
					<button id="password_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
			</td>
			</tr>
						
			<tr>
				<td><a href="#" class="my_tooltip" data-toggle="tooltip" title="Indicate a list of accepted device addresses, e.g. 6,7,0x01020304">source list</a></td>
				<td id="td_waziup_source_list_value">
					<?php 
						$array = $key_clouds['waziup_source_list'];
						$size = count($key_clouds['waziup_source_list']);
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
					<button id="btn_edit_waziup_source_list" type="button" class="btn btn-primary">
						<span class="fa fa-edit"></span>
					</button>
				</td>
				<td id="td_edit_waziup_source_list">
					<div id="div_edit_waziup_source_list" class="form-group">
						<label>Add a sensor to your list</label>
						<input id="waziup_source_list_input" class="form-control" placeholder="e.g. 6,7,0x01020304" type="text"
						value="<?php 
							$array = $key_clouds['waziup_source_list'];
							$size = count($key_clouds['waziup_source_list']);
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
				<td id="td_waziup_source_list_submit" align="right">
					<button id="btn_waziup_source_list_submit" type="submit" class="btn btn-primary">
						Submit
						<span class="fa fa-edit"></span>
					</button>
				</td>
			</tr>
			
			<tr>
			<td>visibility</td>
			<td id="visibility_value"><?php echo $key_clouds['visibility']; ?></td>
			<td align="right"><button id="btn_edit_visibility" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
			<td id="td_edit_visibility">
				<div id="div_visibility_options" class="form-group">
					
					<div class="radio">
					<fieldset id="waziup_visibility_group" >
						<label>
							<input type="radio" name="waziup_visibility_group" id="visibility_public" value="public" <?php if($key_clouds['visibility']=="public") echo "checked"?> >Public
						</label>
						</br>
						<label>
							<input type="radio" name="waziup_visibility_group" id="visibility_private" value="private" <?php if($key_clouds['visibility']=="private") echo "checked"?> >Private
						</label>
						</fieldset>
					</div>
				</div>
			</td> 
			<td id="td_visibility_submit" align="right">
					<button id="btn_visibility_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
			</td>
			</tr>													
			</tbody>
		</table>
		<p>The WAZIUP cloud uses FIWARE platform with the possibility to define domains: the domain will be defined as <tt>project_name+'-'+organization_name+service_tree</tt>, e.g. <tt>waziup-UPPA-TESTS</tt> if <tt>project_name</tt> is <tt>waziup</tt>, <tt>organization_name</tt> is <tt>UPPA</tt> and <tt>service_tree</tt> is <tt>-TESTS</tt> (<tt>service_tree</tt> can be empty otherwise it must begin with a '-'). The device id will be <tt>organization_name+service_tree+"_Sensor"+device_addr</tt>. For instance, for sensor 2 hosted by <tt>UPPA</tt> with a <tt>service_tree</tt> of <tt>-TESTS</tt>, device id will be <tt>UPPA-TESTS_Sensor2</tt>.</p>
		
		<p>The default <tt>organization_name</tt> is 'ORG'. It is <b>necessary</b> to change it to your organization name.</p>
		
		<p><tt>username</tt> is your WAZIUP username. <tt>password</tt> is your WAZIUP password. Use <b>only letters and numbers</b> for <tt>username</tt> and <tt>password</tt>. To create an account on the WAZIUP platform, go to <a href="https://dashboard.waziup.io">https://dashboard.waziup.io</a>.</p>
		
		<p><tt>visibility</tt> can be set to <tt>private</tt> so that the gateway creates private sensors under the account of the user. Private sensors can only be seen by its owner.</p>
		
		<p><b>Do not use space nor '/'</b> in any of these parameters, use '_' or '-' instead.</p>
		
		</div>
	</div>