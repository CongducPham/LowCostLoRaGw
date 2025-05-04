// shorter syntax for document.ready
$(function () {
	var the_node;
	$('#div_time_interval').hide();
	   
//########################################
// Video node selection and time interval
//########################################
   
	$('#select_video_node').on('change', function() {
		the_node = $('#select_video_node').val();
		$.get("process.php", {node: the_node, time_interval: "all"}, function(data){
			$('#div_time_interval').show();     
			$('#select_time_interval').html('<option value="1h">1 hour</option><option value="2h">2 hours</option>'+
  							'<option value="3h">3 hours</option><option value="all" selected>All</option>');       	
			$('#div_pic_by_node').html(data);
        	});
	});	
    
    
    $('#select_time_interval').on('change', function() {	
    	time_interval = $('#select_time_interval').val();

    	$.get("process.php", {node: the_node, time_interval: time_interval}, function(data){
            	$('#div_pic_by_node').html(data);
        });
    });
    
    
    $('#a_mv_img').click(function() {	
    	$.get("process.php", {mv: "true"}, function(data){
            $('#mv_img_msg').html(data);
            setTimeout(function() {
  				$('#mv_img_msg').html("");
	    	},5000);
        });
    });

});
 
    
