<?php

function islandora_pid_list_form(array &$form_state) {
  global $user;
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
  module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');
  
  $pidlisthelper = new Islandora_Pid_List_Helper();
  $pidlist = $pidlisthelper->get_user_pid_list($user->uid);
  
  foreach ($pidlist as $key => $value) {
    $titlefield = array( 
     '#type' => 'fieldset',
     '#collapsible' => TRUE,
     '#collapsed'=> TRUE,
     '#title' => $value->listname . ' [ID:' . $value->listid . ']'
     );
    $form['pidlists'][$key] = $titlefield; 
     
    $userfield = array( 
     '#type' => 'fieldset',
     '#collapsible' => TRUE,
     '#collapsed'=> FALSE,
     '#title' => t('Users'),
    );
    $form['pidlists'][$key]['users'] = $userfield;
     
    foreach ($value->userlist as $itr => $piduser) {
      $tempuser = user_load($piduser);
      $tempuser = $tempuser->name;
      $form['pidlists'][$key]['users'][$itr] = array(
        '#type' => 'item',
        '#value' => $tempuser
        );
    }
  
    $pidfield = array( 
     '#type' => 'fieldset',
     '#collapsible' => TRUE,
     '#collapsed'=> FALSE,
     '#title' => t('Pids'),
    );
    $form['pidlists'][$key]['pids'] = $pidfield;
    
    foreach ($value->pidlist as $itr=> $pid) {
     $form['pidlists'][$key]['pids'][$itr] = array (
       '#prefix' => '<div class=pid'.$itr.'>',
       '#suffix' => '</div>',
       '#value' => $pid
     );
    }
   
    $form['pidlists'][$key]['pids']['addpid'] = array (
      '#type' => 'textfield',
      '#title' => t('PID'),
      '#size' => 30
    );
    
    $form['pidlists'][$key]['pids']['addpidsubmit'] = array (
     '#type' => 'submit',
     '#value' => t('Add pid to list'),
     '#submit' => array('islandora_pid_list_form_add_pid_submit')
      );
 }
 dpm($form_state, "before submit");
 return $form;
}

function islandora_pid_list_form_add_pid_submit(array $form, array &$form_state) {
  //$pid = $form_state['clicked_button']['#pid'];
  //echo $pid;
  dsm($form_state, "form state");
}