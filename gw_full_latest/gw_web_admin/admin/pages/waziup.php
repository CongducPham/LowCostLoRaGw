<?php

?>

	</br>
	<div id="waziup_status_msg"></div>
	
	<div class="col-md-10 col-md-offset-0">
		<div class="table-responsive">
		<table class="table table-striped table-bordered table-hover">
			<thead></thead>
			<tbody>
			<tr>
			<td>Enabled</td>
			<td id="waziup_status_value"><?php cloud_status($clouds, "python CloudWAZIUP.py"); ?></td>
			<td align="right"><button id="btn_edit_waziup_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
			<td id="td_edit_waziup_status">
				<div id="div_waziup_status_options" class="form-group">
					
					<div class="radio">
					<fieldset id="waziup_status_group" >
						<label>
							<input type="radio" name="waziup_status_group" id="waziup_true" value="true" checked>True
						</label>
						</br>
						<label>
							<input type="radio" name="waziup_status_group" id="waziup_false" value="false" >False
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
			<td>organization name</td>
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
			<td>service tree</td>
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
			<td>username</td>
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
			<td>password</td>
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
				<td>source list</td>
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
						<input id="waziup_source_list_input" class="form-control" placeholder="e.g. 6,7,8" type="text"
						value="<?php 
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