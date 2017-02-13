<?php
$block = (object) module_invoke('show_stat', 'block', 'view', '0');
if(isset($block->content)){
print  '<div style="position: absolute !important; top: 50px !important; left: 81% !important; color: grey; line-height:120%; font-size:12px">'. trim($block->content) .'</div>';
}?>
     
before this following line     
      </div> <!-- /header -->