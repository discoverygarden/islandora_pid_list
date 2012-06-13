<?php

function islandora_pid_list_form() {
  global $user;
  global $base_url;
  module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');
  module_load_include('inc', 'islandora_pid_list', 'pidlist');
  module_load_include('inc', 'fedora_repository', 'api/tuque');
  drupal_add_css(drupal_get_path('module', 'islandora_pid_list') . '/css/islandora_pid_list.css');
   
  // Get all users for use in select for forms
  
  $options = array("" => "- Select A User To Add - ");
  $query = db_query("SELECT uid, name FROM {users} ORDER BY uid");
  while ($result = db_fetch_object($query)) {
    if ($result->uid != 0) {
      $options[$result->uid]= $result->uid . ': ' . $result->name;
      }
    }
  
  $pidlist = get_user_pid_list($user->uid);
  
  $form = array(
    '#tree' => TRUE,
  );
  foreach ($pidlist as $key => $value) {
  
    $form[$key] = array( 
     '#type' => 'fieldset',
     '#collapsible' => TRUE,
     '#collapsed'=> TRUE,
     '#title' => $value->listname . ' [ID:' . $value->listid . ']',
     );
    
    $form[$key]['plo'] = array(
      '#type' => 'value',
      '#value' => $value,
    );
    
    $form[$key]['users'] = array( 
     '#type' => 'fieldset',
     '#collapsible' => TRUE,
     '#collapsed'=> FALSE,
     '#title' => t('Users'),
    );
          
    $form[$key]['pids'] = array( 
     '#type' => 'fieldset',
     '#collapsible' => TRUE,
     '#collapsed'=> FALSE,
     '#title' => t('Pids'),
    );
    
    $fku =& $form[$key]['users'];
    $fkp =& $form[$key]['pids']; 
        
    foreach ($value->userlist as $itr => $piduser) {
      $tempuser = user_load($piduser);
      
      $fku[$itr] = array(
        '#prefix' => '<div class="users">',
        '#suffix' => '</div>',
        'label' => array(
          '#type' => 'item',
          '#value' => $tempuser->name,
        ),
        'user' => array(
          '#type' => 'value',
          '#value' => $piduser,
        ),
        'list-id' => array(
          '#type' => 'value',
          '#value' => $key,
        ),
        'remove' => array (
          '#type' => 'image_button',
          '#src' => drupal_get_path('module', 'fedora_repository') . '/images/purge_big.png',
        ),
      );
    }
    
    $fku['listusers'] = array (
        '#type' => 'select',
        '#default value' => t('Select a user to add'),
        '#title' => t('Add a user to the list'),
        '#options' => $options,
        );
   if (!empty($value->pidlist)) {
    foreach ($value->pidlist as $itr => $pid) {
     $fedoraobject = islandora_object_load($pid);
     $pidvalue = $pid;
     
     if ($fedoraobject) {
       $pidvalue = l($pid, 'fedora/repository/' . $pid, array(
         'attributes' => array(
           'target' => '_blank'
          )));
     }
     
     $fkp[$itr] = array(
       '#prefix' => '<div class="pids">',
       '#suffix' => '</div>',
       'label' => array (
         '#type' =>'item',
         '#value' => $pidvalue,
       ),
       'pid' => array(
         '#type' => 'value',
         '#value' => $pid,
       ),
       'remove' => array (
        '#type' => 'image_button',
        '#src' => drupal_get_path('module', 'fedora_repository') . '/images/purge_big.png',  
       ),
       'list-id' => array(
          '#type' => 'value',
          '#value' => $key,
       ),
     );
   }
  } 
     $fkp['pidtext'] = array (
      '#type' => 'textfield',
      '#title' => t('Add a PID'),
      '#size' => 30,
     );
 }
 
  $form['submit'] = array (
      '#type' => 'submit',
      '#name' => 'islandora_pid_list_form_submit_button',
      '#value' => t('Submit'),
    );
  
  return $form;
}

function islandora_pid_list_form_submit(array $form, array &$form_state) {
  $tempparents = $form_state['clicked_button']['#parents'];
  $topparent = array_pop($tempparents);
  
  if ($topparent !== 'submit') {
   
  $parents = islandora_pid_list_form_get_parents($form_state);
  $listobj = $form_state['values'][$parents['list-id']]['plo'];
    
    if(array_key_exists('pid',$parents)) {
      $listobj->remove_pids(array($parents['pid']));
    }
  
    else {
      $listobj->remove_users(array($parents['user']));
    }
  
  /*if ($form_state['clicked_button']['#array_parents'][0] !== 'submit') {
    dpm($form_state, 'form ');
    $listindex = $form_state['clicked_button']['#array_parents'][0];
    
    $removeindex = $form_state['clicked_button']['#array_parents'][2];
    
    if ($form_state['clicked_button']['#array_parents'][1] === 'pids') {
      $listobj->remove_pids(array($listobj->pidlist[$removeindex]));
    }
    else {
      $listobj->remove_users(array($listobj->userlist[$removeindex]));
    }
  }*/
  }
  else {
  // Add user input to lists
    foreach($form_state['values'] as $key) {
      if (is_array($key)) {
        $object = $key['plo'];
        $userinput = array();
        
        if (!empty($key['users']['listusers'])) {
          $userinput[] = $key['users']['listusers'];
        }
        
        $pidinput = array();
        if (!empty($key['pids']['pidtext'])) {
          $pidinput[] = $key['pids']['pidtext'];
        }
            
      // Add to list
        if (!empty($userinput))
        {
          if (!in_array($userinput[0],$object->userlist)) {
          $object->add_users($userinput);
          }
        else {
          drupal_set_message(t("The user '" . $userinput[0] . "' already belongs to '" . $object->listname . "'"),'error');
        }
       }
      
       if (!empty($pidinput)) {
        if (!in_array($pidinput[0],$object->pidlist)) {
          try {
              $object->add_pids($pidinput);
              } 
          catch (Exception $e) {
              drupal_set_message($e->getMessage(),'error');
          }
        }
        else {
          drupal_set_message(t("The pid '" . $pidinput[0] . "' already belongs to '" . $object->listname . "'"),'error');  
        }
      }
    }
  }
 }
}

function islandora_pid_list_form_get_parents($form_state) {
 $parents = $form_state['clicked_button']['#array_parents'];
 $parents = array_slice($parents, 0, -1);
 $temp = $form_state['values'][array_shift($parents)];
 foreach ($parents as $parent) {
  $temp = $temp[$parent];
 }
 return $temp;
}

