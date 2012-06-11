<?php

function islandora_pid_list_forms($form_id, $args) {
  $forms = array();
  
  if (strpos($form_id, 'islandora_pid_list_form_') !== FALSE) {
    $forms[$form_id] = array (
      'callback' => 'islandora_pid_list_form',
      'callback arguments' => array($form_id, $args),
    );
  } 
 return $forms;
}

function islandora_pid_list_form($form_state, $form_id, $args) {
  $value = $args[0][0];
  $key = $args[0][1];
  
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
  
  
  
  $titlefield = array( 
     '#type' => 'fieldset',
     '#collapsible' => TRUE,
     '#collapsed'=> TRUE,
     '#title' => $value->listname . ' [ID:' . $value->listid . ']',
     '#tree' => TRUE,
     );
    $form[$key] = $titlefield; 
     
    $pidlistobj = array(
    '#type' => 'value',
    '#value' => $value,
    );
  
  $form[$key]['plo'] = $pidlistobj;
    
    $userfield = array( 
     '#type' => 'fieldset',
     '#collapsible' => TRUE,
     '#collapsed'=> FALSE,
     '#title' => t('Users'),
    );
    $form[$key]['users'] = $userfield;
     
    foreach ($value->userlist as $itr => $piduser) {
      $tempuser = user_load($piduser);
      $tempuser = $tempuser->name;
      $form[$key]['users'][$itr] = array(
        '#type' => 'item',
        '#value' => $tempuser,
        );
    }
  
    $pidfield = array( 
     '#type' => 'fieldset',
     '#collapsible' => TRUE,
     '#collapsed'=> FALSE,
     '#title' => t('Pids'),
    );
    $form[$key]['pids'] = $pidfield;
    
    foreach ($value->pidlist as $itr => $pid) {
     $form[$key]['pids'][$itr] = array (
       '#prefix' => '<div class="pid'.$itr.'">',
       '#suffix' => '</div>',
       '#value' => $pid,
     );
    }
   
    $form[$key]['pids']['pidtext'] = array (
      '#name' => 'textfield for pidlist ' . $key,
      '#type' => 'textfield',
      '#title' => t('PID'),
      '#size' => 30,
    );
    
    $form[$key]['pids']['addpidsubmit'] = array (
     '#name' => 'submit for pidlist ' . $key,
     '#type' => 'submit',
     '#value' => t('Add pid to list'),
     '#submit' => array('islandora_pid_list_form_submit'), 
    );
       
    return $form;
    
}


function islandora_pid_list_form_submit(array $form, array &$form_state) {
  dpm($form, 'form');
  dpm(&$form_state, 'form state');
  dpm($_POST, 'post');
 }

function islandora_pid_list_view() {
  global $user;
  module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');
  $pidlisthelper = new Islandora_Pid_List_Helper();
  $pidlist = $pidlisthelper->get_user_pid_list($user->uid);
  
  
  foreach ($pidlist as $key => $value) {
    $output .= drupal_get_form('islandora_pid_list_form_'.$key, array($value,$key));
  }
   return $output;
}