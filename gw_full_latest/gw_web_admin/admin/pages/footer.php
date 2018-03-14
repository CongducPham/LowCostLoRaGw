<?php

?>

        <!-- Reboot dialog --> 
        <div class="modal fade" id="confirm-reboot" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <div class="modal-header">
                    <button type="button" class="close" data-dismiss="modal" aria-hidden="true">×</button>
                    <h4 class="modal-title" id="myModalLabel">Confirm Reboot</h4>
                </div>
                <div class="modal-body">
                    <p>You are about to reboot the gateway.<b><i class="title"></i></b></p>
                    <p>Note that you have to wait a while and then refresh the page.</p>
                    <p>Do you want to proceed?</p>
                </div>
                <div class="modal-footer">
                    <button id="cancel_reboot" type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                    <button id="ok_reboot" type="button" class="btn btn-danger btn-ok">Reboot</button>
                </div>
            </div>
       </div>
       </div>
       
       <!-- Shutdown dialog --> 
        <div class="modal fade" id="confirm-shutdown" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <div class="modal-header">
                    <button type="button" class="close" data-dismiss="modal" aria-hidden="true">×</button>
                    <h4 class="modal-title" id="myModalLabel">Confirm Shutdown</h4>
                </div>
                <div class="modal-body">
                    <p>You are about to shutdown the gateway.<b><i class="title"></i></b></p>
                    <p>Note that physical access to gateway is needed to power on it again.</p>
                    <p>Do you want to proceed?</p>
                </div>
                <div class="modal-footer">
                    <button id="cancel_shutdown" type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                    <button id="ok_shutdown" type="button" class="btn btn-danger btn-ok">Shutdown</button>
                </div>
            </div>
       </div>
       </div>
       
        </div>
        <!-- /#page-wrapper -->             
    </div>
    <!-- /#wrapper -->
    
    <!-- jQuery -->
    <script src="../vendor/jquery/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="../vendor/bootstrap/js/bootstrap.min.js"></script>

    <!-- Metis Menu Plugin JavaScript -->
    <script src="../vendor/metisMenu/metisMenu.min.js"></script>

    <!-- Morris Charts JavaScript -->
    <script src="../vendor/raphael/raphael.min.js"></script>
    <!--
    <script src="../vendor/morrisjs/morris.min.js"></script>
    <script src="../data/morris-data.js"></script>-->

    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

</body>

</html>
