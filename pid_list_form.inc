<?php
module_load_include('inc', 'islandora_pid_list', 'islandora_pid_list.api');
module_load_include('inc', 'islandora_pid_list', 'pidlist');
module_load_include('inc', 'fedora_repository', 'api/tuque');
drupal_add_css(drupal_get_path('module', 'islandora_pid_list') . '/css/islandora_pid_list.css');

function islandora_pid_list_settings() {
  $form = array();
  $form['islandora_pid_list_show_tabs'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show Pid List Tabs'),
    '#description' => t("Show a tab for the PID lists associated to each object on each Fedora object's page."),
    '#default_value' => variable_get('islandora_pid_list_show_tabs', TRUE),
  );
  return system_settings_form($form);
}

function islandora_pid_list_list_form($form_state) {
  global $user;
  // Get all users for use in select for forms
  $query = db_query("SELECT uid, name FROM {users} ORDER BY uid", $user->uid);
  
  while ($result = db_fetch_object($query)) {
    $resultname = $result->name;
      if ($result->uid == 0) {
        $resultname = 'Anonymous';
      }
    $options[$result->uid] = array (
      'uid' => $result->uid,
      'name' => $resultname
    );
  }
  
  $pidlist = get_user_pid_list(array('user' => $user->uid));
  $form = array(
    '#tree' => TRUE,
  );
  
  // Needed to handle the press of enter key for submits
  $form['enter_submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save',
    '#attributes' => array('class' => 'enter_submit'),
  );

  $form['userlists'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => t('My Lists'),
  );
  
  if (!empty($pidlist)) {
    foreach ($pidlist as $key => $value) {
      $idoutput = str_replace('session_', '', $value->listid);
      $form['userlists'][$key] = array( 
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed'=> TRUE,
        '#title' => $value->listname . ' [ID:' . $idoutput . ']',        
      );
        if (user_access(t("Manage other users' pidlists")) || $user->uid == $value->listowner) {
          $form['userlists'][$key]['listname'] = array (
            '#type' => 'textfield',
            '#title' => t('List Name'),
            '#size' => 20,
            '#default_value' => $value->listname,
            '#prefix' => '<div class="listdetails">',
          );
          
          $form['userlists'][$key]['remove'] = array (
            '#type' => 'image_button',
            '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png', 
            '#suffix' => '</div>'
          );
        }
      
        $form['userlists'][$key]['list-id'] = array(
            '#type' => 'value',
            '#value' => $key,
        );  
        $form['userlists'][$key]['plo'] = array(
          '#type' => 'value',
          '#value' => $value,
        );
       
      $fku =& $form['userlists'][$key]['users'];
      
      if (user_access(t("Manage other users' pidlists")) || $user->uid == $value->listowner) {
        $fku = array( 
          '#type' => 'fieldset',
          '#collapsible' => TRUE,
          '#collapsed'=> FALSE,
          '#title' => t('Users'),
        );
        
        if (count($value->userlist) != 1) {
          $uindex = array_search($user->uid, $value->userlist, TRUE);
          $fiuser = $value->userlist[0];
          $value->userlist[0] = $value->userlist[$uindex];
          $value->userlist[$uindex] = $fiuser;
        }
        foreach ($value->userlist as $itr => $piduser) {
          $tempuser = user_load($piduser);
          if ($tempuser->uid == $user->uid && $user->uid != 0)
          {
            $tempuser = t('Yourself');
          }
          elseif ($tempuser->uid == 0) {
            $tempuser = t('Anonymous');
          }
          else {
            $tempuser = $tempuser->name;
          }

          $fku[$itr] = array(
            '#prefix' => '<div class="users">',
            '#suffix' => '</div>',
            'label' => array(
              '#type' => 'item',
              '#value' => $tempuser,
            ),
            'user' => array(
              '#type' => 'value',
              '#value' => $piduser,
            ),
            'plo' => array(
              '#type' => 'value',
              '#value' => $value,
            ),
            'list-id' => array(
              '#type' => 'value',
              '#value' => $key,
            ),
            'remove' => array (
              '#type' => 'image_button',
              '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png',
            ),
           );    
      }
      
      $selectoptions = array("" => "- Select A User To Add - ");
      
      foreach($options as $index => $item) {
        if (!in_array($index,$value->userlist)) {
        $selectoptions[$index] = $item['uid'] . ': ' . $item['name'];
        }  
      }
      $fku['listusers'] = array (
        '#type' => 'select',
        '#default value' => t('Select a user to add'),
        '#title' => t('Add a user to the list'),
        '#options' => $selectoptions,
        '#width' => 20,
      );
    }
     
    $form['userlists'][$key]['pids'] = array( 
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed'=> FALSE,
      '#title' => t('Pids'),
    );
    $fkp =& $form['userlists'][$key]['pids']; 
    
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
          'plo' => array(
              '#type' => 'value',
              '#value' => $value,
          ),
          'list-id' => array(
              '#type' => 'value',
              '#value' => $key,
          ),
        );
        if (user_access(t("Manage other users' pidlists")) || $user->uid == $value->listowner) {
          $fkp[$itr]['remove'] = array (
            '#type' => 'image_button',
            '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png',  
          );
        }
      }
    }
    else {
      $fkp['empty'] = array(
        '#prefix' => '<div class="pids">',
        '#suffix' => '</div>',
        '#value' => t('No pids currently associated'),
      );
    }
     if (user_access(t("Manage other users' pidlists")) || $user->uid == $value->listowner) { 
      $fkp['pidtext'] = array (
        '#type' => 'textfield',
        '#title' => t('Add a PID'),
        '#size' => 20,
      );
     } 
  }
 }
 else {
   $form['userlists']['empty'] = array (
     '#type' => 'item',
     '#value' => t('No lists currently associated'),
     '#prefix' => '<div class="emptylist">',
     '#suffix' => '</div>',
   );
 }
     
 if (user_access(t('Adminisiter islandora_pid_list module'))) {
    $templists = get_user_pid_list(array('all' => TRUE));
    $otherlists = array();
    foreach($templists as $item) {
      if (!in_array($user->uid,$item->userlist)) {
        $otherlists[] = $item;
      }
    }
    $form['alllists'] = array(
      '#type' => 'fieldset',
      '#title' => t('All Other Lists'),
      '#collapsible' => TRUE,
      '#collapsed'=> FALSE,
    );
    if (!empty($otherlists)) {
      foreach ($otherlists as $key => $value) {
        $form['alllists'][$key] = array( 
          '#type' => 'fieldset',
          '#collapsible' => TRUE,
          '#collapsed'=> TRUE,
          '#title' => $value->listname . ' [ID:' . $value->listid . ']',        
        );
      $form['alllists'][$key]['listname'] = array (
        '#type' => 'textfield',
        '#title' => t('List Name'),
        '#size' => 20,
        '#default_value' => $value->listname,
        '#prefix' => '<div class="listdetails">',
      );

      $form['alllists'][$key]['remove'] = array (
        '#type' => 'image_button',
        '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png', 
        '#suffix' => '</div>'
      );

      $form['alllists'][$key]['list-id'] = array(
        '#type' => 'value',
        '#value' => $key,
      );  
      $form['alllists'][$key]['plo'] = array(
        '#type' => 'value',
        '#value' => $value,
      );

      $fku =& $form['alllists'][$key]['users'];

      if (user_access(t("Manage other users' pidlists")) || $user->uid == $value->listowner) {
        $fku = array( 
          '#type' => 'fieldset',
          '#collapsible' => TRUE,
          '#collapsed'=> FALSE,
          '#title' => t('Users'),
        );

        if (count($value->userlist) != 1) {
          $uindex = array_search($user->uid, $value->userlist, TRUE);
          $fiuser = $value->userlist[0];
          $value->userlist[0] = $value->userlist[$uindex];
          $value->userlist[$uindex] = $fiuser;
        }
        foreach ($value->userlist as $itr => $piduser) {
          $tempuser = user_load($piduser);
            if ($tempuser->uid == 0) {
              $tempuser = t('Anonymous');
            }
            else {
              $tempuser = $tempuser->name;
            }

            $fku[$itr] = array(
              '#prefix' => '<div class="users">',
              '#suffix' => '</div>',
              'label' => array(
                '#type' => 'item',
                '#value' => $tempuser,
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
                '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png',
              ),
              'plo' => array(
                '#type' => 'value',
                '#value' => $value,
              )
            );    
        }
        $selectoptions = array("" => "- Select A User To Add - ");
        foreach($options as $index => $item) {
          if (!in_array($index,$value->userlist)) {
          $selectoptions[$index] = $item['uid'] . ': ' . $item['name'];
          }  
        }
        $fku['listusers'] = array (
          '#type' => 'select',
          '#default value' => t('Select a user to add'),
          '#title' => t('Add a user to the list'),
          '#options' => $selectoptions,
          '#width' => 20,
        );


      $form['alllists'][$key]['pids'] = array( 
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed'=> FALSE,
        '#title' => t('Pids'),
      );
      $fkp =& $form['alllists'][$key]['pids']; 

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
              'list-id' => array(
                '#type' => 'value',
                '#value' => $key,
              ),
              'plo' => array(
                '#type' => 'value',
                '#value' => $value,
              )
            );

            $fkp[$itr]['remove'] = array (
              '#type' => 'image_button',
              '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png',  
            );
          }
        }
        else {
          $fkp['empty'] = array(
            '#prefix' => '<div class="pids">',
            '#suffix' => '</div>',
            '#value' => t('No pids currently associated'),
          );
        }
          $fkp['pidtext'] = array (
            '#type' => 'textfield',
            '#title' => t('Add a PID'),
            '#size' => 20,
          );
      }
    }
   }
   else {
    $form['alllists']['empty'] = array (
      '#type' => 'item',
      '#value' => t('No lists currently associated'),
      '#prefix' => '<div class="emptylist">',
      '#suffix' => '</div>',
    );  
   }
 }
  $form['submit'] = array (
      '#type' => 'submit',
      '#name' => 'islandora_pid_list_list_form_submit_button',
      '#value' => t('Submit'),
  );
  $form['clear'] = array (
      '#type' => 'submit',
      '#name' => 'islandora_pid_list_list_form_clear_button',
      '#value' => t('Clear'),
  );
  return $form;
}

function islandora_pid_list_list_form_submit(array $form, array &$form_state) {
  global $user;
  
  $tempparents = $form_state['clicked_button']['#parents'];
  $topparent = array_pop($tempparents);
  
  if ($topparent !== 'submit' && $topparent !== 'clear' && $topparent !== 'enter_submit') {
    $parents = islandora_pid_list_list_form_get_parents($form_state);
    $listobj = $parents['plo'];
   
    if(array_key_exists('pid',$parents)) {
      $listobj->remove_pids(array($parents['pid']));
    }
  
    elseif(array_key_exists('user',$parents)) {
      if (count($listobj->userlist) == 1) {
        $listobj->remove_list();
      }
      else {
       $listobj->remove_users(array($parents['user']));
      }
    }
    else {
      $listobj->remove_list();
    }
  }
  elseif ($topparent === 'submit' || $topparent === 'enter_submit') {
  // Add user input to lists
    islandora_pid_list_save_changes('userlists',$form_state);
    
    if (user_access(t('Adminisiter islandora_pid_list module')) && array_key_exists('alllists',$form_state['values'])) {
      islandora_pid_list_save_changes('alllists',$form_state);
    }
 
  }
}

function islandora_pid_list_save_changes($param, $form_state) {
  global $user;
    
  foreach($form_state['values'][$param] as $key) {
    if (is_array($key)) {
      $object = $key['plo'];
      $userinput = array();
        if (user_access(t("Manage other users' pidlists")) || $user->uid == $object->listowner) {
          if ($object->listname != $key['listname']) {
            if (ctype_space($key['listname']) || $key['listname'] == '') {
              drupal_set_message(t('The list %listname must have a name!',
                  array (
                  '%listname' => $object->listname,  
                  )
            ),'error');
            }
            else {
              $object->change_list_name($key['listname']);
            }
          }
        if (strlen($key['users']['listusers'])) {
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
          drupal_set_message(t("The user %userinput already belongs to %listname", 
              array(
                '%userinput' => user_load($userinput[0])->name, 
                '%listname' => $object->listname,
              )
          ),'error');
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
          drupal_set_message(t("The pid %pidinput already belongs to %listname",
              array (
                '%pidinput' => $pidinput[0],
                '%listname' => $object->listname,
                )
          ),'error');  
        }
      }
     }
    }
   }
}


function islandora_pid_list_list_form_get_parents($form_state) {
 $parents = $form_state['clicked_button']['#array_parents'];
 $parents = array_slice($parents, 0, -1);
 $temp = $form_state['values'][array_shift($parents)];
  foreach ($parents as $parent) {
    $temp = $temp[$parent];
  }
 return $temp;
}

function islandora_pid_list_add_form($form_state) {
  global $user;
  
  $redirectpid = request_uri();
  $redirectpid = explode("?",$redirectpid);
  $redirectpid = $redirectpid[1];
  
  $options = array("" => "- Select A User To Add - ");
  $query = db_query("SELECT uid, name FROM {users} WHERE uid != %d ORDER BY uid", $user->uid);
  while ($result = db_fetch_object($query)) {
    $resultname = $result->name;
      if ($result->uid == 0) {
        $resultname = 'Anonymous';
      }
      $options[$result->uid]= $result->uid . ': ' . $resultname;
  }
    
  $form = array (
    '#title' => t('Add'),
      '#tree' => TRUE,
      'listname' => array(
        '#type' => 'textfield',
        '#title' => t('List Name'),
        '#size' => 20,  
      ),
      'listusers' => array (
        '#type' => 'select',
        '#default value' => t('Select a user to add'),
        '#title' => t('Add a user to the list'),
        '#options' => $options,
      ),
      'pidtext' => array (
        '#type' => 'textfield',
        '#title' => t('Add a PID'),
        '#size' => 20,
      ),
    );
  
   
  if (!empty($redirectpid)) {
    $form['redirect'] = array(
        '#type' => 'value',
        '#value' => $redirectpid,
    );
    
    $form['pidtext']['#default_value'] = $redirectpid;
  }
  
  $form['submit'] = array (
      '#type' => 'submit',
      '#name' => 'islandora_pid_list_add_form_submit_button',
      '#value' => t('Submit'),
  );
  $form['clear'] = array (
      '#type' => 'submit',
      '#name' => 'islandora_pid_list_add_form_clear_button',
      '#value' => t('Clear'),
  );
 return $form; 
}

function islandora_pid_list_add_form_submit(array $form, array &$form_state) {
  global $user;
  $createname = $form_state['values']['listname'];
  $createpid = $form_state['values']['pidtext'];
  $createuser = $form_state['values']['listusers'];
  $redirectpid = $form_state['values']['redirect'];
  // Create a new list
  if (!ctype_space($createname) && $createname != '') {
    $paramusers = array();
    $parampids = array();

    $params = array(
      'retrieve' => FALSE,
      'useruid' => $user->uid,
      'listowner' => $user->uid,
      'listname' => $createname,
      'listusers' => array($user->uid),
    );

    if ($user->uid == 0)
    {
      $params['session'] = TRUE;
    }

    $templist = new PidList($params);

    if (!empty($createuser) && $createuser != $user->uid) {
      $paramusers[] = $createuser;
      $templist->add_users($paramusers);
    }

    if (!empty($createpid)) {
      try {
          $templist->add_pids(array($createpid));
          } 
      catch (Exception $e) {
        drupal_set_message($e->getMessage() . ' The list was still created!','error');
      }
    }
    if ($user->uid == 0) {
      $savedlist = 'Note that lists created when not logged will not be saved.';
    }
    drupal_set_message(t('The new list %listname was created. %saved',  
        array(
          '%listname' =>$createname,
          '%saved' => $savedlist,
          )
        ));
    if (empty($redirectpid)) {
      drupal_goto('pidlist');
    }
    else {
      $redirect = 'fedora/repository/' . $redirectpid;
      drupal_goto($redirect);
      
    }
  }
  else {
    drupal_set_message(t('A new list must at least have a name!'), 'error');
    drupal_goto('pidlist/add',$redirectpid);
  }
 }

  function islandora_pid_list_pid_form($form_state, $pid) {
    global $user;
    $pidlist = array();
    $pidlist = get_user_pid_list(array('pid' => $pid));
    

    $form = array (
      '#title' => t('Lists'),
      '#tree' => TRUE,
      'userlists' => array(
        '#type' => 'fieldset',
        '#title' => t('My Lists'),
        '#collapsible' => TRUE,
        '#collapsed'=> FALSE,
      ),
      'pid' => array(
        '#type' => 'value',
        '#value' => $pid,
      ), 
    );

    $listids = array();
    $selectlist = array();

    // $form['userlists'] = user associated lists (ie lists the user can see).
    foreach ($pidlist as $key => $value) {
      if (in_array($user->uid, $value->userlist)) {
        $idoutput = str_replace('session_', '', $value->listid);
        $form['userlists'][$key]['label'] = array( 
            '#type' => 'item',
            '#title' => $value->listname . ' [ID:' . $idoutput . ']',  
            '#prefix' => '<div class="userlists">',
        );
        if (user_access(t("Manage other users' pidlists")) || $user->uid == $value->listowner) {
          $form['userlists'][$key]['remove'] = array (
            '#type' => 'image_button',
            '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png', 
            '#suffix' => '</div>',
          );
        }
        $form['userlists'][$key]['list-id'] = array(
          '#type' => 'value',
          '#value' => $key
        );  

        $form['userlists'][$key]['listtype'] = array(
          '#type' => 'value',
          '#value' => 'userlists',
        );

        $form['userlists'][$key]['plo'] = array(
        '#type' => 'value',
        '#value' => $value,
        );
        array_push($listids, $value->listid);
      }
    }
    if (empty($pidlist)) {
       $form['userlists']['empty'] = array(
        '#value' => t('No lists currently associated'),
        '#prefix' => '<div class="emptypid">',
        '#suffix' => '</div>',
       );  
     } 

    $result = db_query('SELECT listid FROM {islandora_pid_list_lists} WHERE uid = %d ORDER BY listid', $user->uid);

    while ($listid = db_fetch_array($result)) {
      if (!in_array($listid['listid'],$listids)) {
        array_push($selectlist, $listid['listid']);
      }
    }
        
    $options['default'] = "- Select A List To Add - ";
    $options['create'] = 'Create a new list';
    foreach ($selectlist as $value) {
      $query = db_query('SELECT listname FROM {islandora_pid_list_names} WHERE listid = %d', $value);
      $result = db_fetch_array($query);
      $options[$value] = $value . ': ' . $result['listname'];
    }
    
    // Grab from session as well if user is anon and exists session objs
    if ($user->uid == 0 && !empty($_SESSION['islandora_pid_list'])) {
      foreach($_SESSION['islandora_pid_list'] as $value) {
        $sessionlist = unserialize($value);
        $output = str_replace('session_', '', $sessionlist->listid);
          if (!in_array($sessionlist->listid,$listids)) {
            $options[$sessionlist->listid] = $output . ': ' . $sessionlist->listname;
          }
      }
    }
        
    $form['userlists']['listoptions'] = array (
          '#type' => 'select',
          '#default value' => t('Select a list to add the object to'),
          '#options' => $options,
          '#prefix' => '<div class="addlist">',
          '#suffix' => '</div>',
        );

    if (user_access(t('Adminisiter islandora_pid_list module'))) {
      $form['alllists'] = array(
        '#type' => 'fieldset',
        '#title' => t('All Other Lists associated to this PID'),
        '#collapsible' => TRUE,
        '#collapsed'=> FALSE,
      );
     foreach ($pidlist as $key => $value) {
      if (!in_array($user->uid, $value->userlist)) {

        $form['alllists'][$key]['label'] = array( 
          '#type' => 'item',
          '#title' => $value->listname . ' [ID:' . $value->listid . ']',  
          '#prefix' => '<div class="userlists">',
        );
        $form['alllists'][$key]['remove'] = array (
          '#type' => 'image_button',
          '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png', 
          '#suffix' => '</div>',
        );

        $form['alllists'][$key]['list-id'] = array(
          '#type' => 'value',
          '#value' => $key
        );  

        $form['alllists'][$key]['plo'] = array(
          '#type' => 'value',
          '#value' => $value,
          );
       }
      }
     if (empty($pidlist)) {
       $form['alllists']['empty'] = array(
        '#value' => t('No lists currently associated'),
        '#prefix' => '<div class="emptypid">',
        '#suffix' => '</div>',
       );  
     } 
   }

    $form['submit'] = array (
        '#type' => 'submit',
        '#name' => 'islandora_pid_list_pid_form_submit_button',
        '#value' => t('Submit'),
    );
    $form['clear'] = array (
        '#type' => 'submit',
        '#name' => 'islandora_pid_list_pid_form_clear_button',
        '#value' => t('Clear'),
    );
  return $form;
}

function islandora_pid_list_pid_form_submit(array $form, array &$form_state) {
  $parents = islandora_pid_list_list_form_get_parents($form_state);
  $tempparents = $form_state['clicked_button']['#parents'];
  $topparent = array_pop($tempparents);
 
  if ($topparent !== 'submit' && $topparent !== 'clear') {
    $parents = islandora_pid_list_list_form_get_parents($form_state);
    $listobj = $parents['plo'];
    $listobj->remove_pids(array($form_state['values']['pid']));
  }
  elseif ($topparent === 'submit') {
    $listid = $form_state['values']['userlists']['listoptions'];
    
    if ($listid != 'default' && $listid != 'create') {
      $list = get_pid_list_by_number($listid);
      $list->add_pids(array($form_state['values']['pid']));
    }
    elseif ($listid == 'create') {
      drupal_goto('pidlist/add', $form_state['values']['pid']);
    }
  }
}