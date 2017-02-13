<?php
// $Id:$

/**
 * @file
 * Provides the administration page for DAE Data browsing.
 */
  
/**
 * Administration settings page
 */ 
function dae_data_admin() {
  $form = array();

  return system_settings_form($form);
}
