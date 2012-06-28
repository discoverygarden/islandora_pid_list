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
  module_load_include('inc', 'php_lib', 'Ahah');
  global $user;
  
  Ahah::get_ahah_js();
  // Get all users for use in select for forms
  $query = db_query("SELECT uid, name FROM {users} ORDER BY uid", $user->uid);
  
  while ($result = db_fetch_object($query)) {
    $resultname = $result->name;
      if ($result->uid == 0) {
        $resultname = 'Anonymous';
      }
    $options[$result->uid] = array(
      'uid' => $result->uid,
      'name' => $resultname
    );
  }
  
  $pidlist = get_user_pid_list(array('user' => $user->uid));
  $form = array(
    '#tree' => TRUE,
    '#prefix' => '<div id="allpidlists">',
    '#suffix' => '</div>',
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
        '#collapsed' => TRUE,
        '#title' => check_plain($value->listname . ' [ID:' . $idoutput . ']'),
      );
        if (user_access("Manage other users' pidlists") || $user->uid == $value->listowner) {
          $form['userlists'][$key]['listname'] = array(
            '#type' => 'textfield',
            '#title' => t('List Name'),
            '#size' => 20,
            '#value' => $value->listname,
            '#prefix' => '<div class="listdetails">',
            '#ahah' => array(
              'keypress' => TRUE,
              'path' => "pidlist/ahah/updatename",
              'wrapper' => "allpidlists",
              'method' => 'replace',          
            ),
          );
          
          $form['userlists'][$key]['changename'] = array(
           '#type' => 'image_button',
            '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/add.png', 
            '#suffix' => '</div>',
            '#ahah' => array(
                'event' => 'click',
                'path' => "pidlist/ahah/updatename",
                'wrapper' => "allpidlists",
                'method' => 'replace',
                ),
          );  
          
          $form['userlists'][$key]['labelremove'] = array(
            '#type' => 'item',
            '#title' => t('Delete this list'),
            '#weight' => 20000,
            '#prefix' => '<div class="removelist">',
          );
          
          $form['userlists'][$key]['remove'] = array(
            '#type' => 'image_button',
            '#weight' => 20001,
            '#src' => drupal_get_path('module', 'fedora_repository') . '/images/purge.gif', 
            '#suffix' => '</div>',
            '#ahah' => array(
                'event' => 'click',
                'path' => "pidlist/ahah/remove/list",
                'wrapper' => "allpidlists",
                'method' => 'replace',
                ),
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
      
      if (user_access("Manage other users' pidlists") || $user->uid == $value->listowner) {
        $fku = array( 
          '#type' => 'fieldset',
          '#collapsible' => TRUE,
          '#collapsed' => FALSE,
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
          if ($tempuser->uid == $user->uid && $user->uid != 0) {
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
            'remove' => array(
              '#type' => 'image_button',
              '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png',
              '#ahah' => array(
                'event' => 'click',
                'path' => "pidlist/ahah/remove/user",
                'wrapper' => "allpidlists",
                'method' => 'replace',
                ),
           ),
         );   
      }
      
      $selectoptions = array("" => "- Select A User To Add - ");
      
      foreach ($options as $index => $item) {
        if (!in_array($index, $value->userlist)) {
        $selectoptions[$index] = $item['uid'] . ': ' . $item['name'];
        }  
      }
      $fku['listusers'] = array(
        '#type' => 'select',
        '#default value' => t('Select a user to add'),
        '#title' => t('Add a user to the list'),
        '#options' => $selectoptions,
        '#width' => 20,
        '#weight' => 10000,
        '#prefix' => '<div class="addusertolist">',
      );
       $fku['listadd'] = array(
        '#type' => 'image_button',
        '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/add.png', 
        '#suffix' => '</div>',
        '#weight' => 10001, 
        '#ahah' => array(
          'event' => 'click',
          'path' => "pidlist/ahah/add/user",
          'wrapper' => "allpidlists",
          'method' => 'replace',
        ),
      ); 
    }
     
    $form['userlists'][$key]['pids'] = array( 
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
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
          'label' => array(
            '#type' => 'item',
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
        if (user_access("Manage other users' pidlists") || $user->uid == $value->listowner) {
          $fkp[$itr]['remove'] = array(
            '#type' => 'image_button',
            '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png',
            '#ahah' => array(
                'event' => 'click',
                'path' => "pidlist/ahah/remove/pid",
                'wrapper' => "allpidlists",
                'method' => 'replace',
                ),
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
     if (user_access("Manage other users' pidlists") || $user->uid == $value->listowner) { 
      $fkp['pidtext'] = array(
        '#type' => 'textfield',
        '#title' => t('Add a PID'),
        '#size' => 20,
        '#prefix' => '<div class="addpid">',
        '#suffix' => '</div>',
        '#weight' => 10000,
        '#ahah' => array(
          'keypress' => TRUE,
          'path' => "pidlist/ahah/add/pid",
          'wrapper' => "allpidlists",
          'method' => 'replace',          
        ),
      );
      
      $fkp['add_button'] = array(
        '#type' => 'image_button',
        '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/add.png', 
        '#weight' => 10001,  
        '#ahah' => array(
          'event' => 'click',
          'path' => "pidlist/ahah/add/pid",
          'wrapper' => "allpidlists",
          'method' => 'replace',
        ),
      );
    } 
  }
 }
 else {
   $form['userlists']['empty'] = array(
     '#type' => 'item',
     '#value' => t('No lists currently associated'),
     '#prefix' => '<div class="emptylist">',
     '#suffix' => '</div>',
   );
 }
     
 if (user_access('Adminisiter islandora_pid_list module')) {
    $templists = get_user_pid_list(array('all' => TRUE));
    
    $otherlists = array();
    foreach ($templists as $item) {
      if (!in_array($user->uid, $item->userlist)) {
        $otherlists[] = $item;
      }
    }
    $form['alllists'] = array(
      '#type' => 'fieldset',
      '#title' => t('All Other Lists'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );
    if (!empty($otherlists)) {
      foreach ($otherlists as $key => $value) {
        $form['alllists'][$key] = array( 
          '#type' => 'fieldset',
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
          '#title' => check_plain($value->listname . ' [ID:' . $value->listid . ']'),  
        );
        $form['alllists'][$key]['listname'] = array(
          '#type' => 'textfield',
          '#title' => t('List Name'),
          '#size' => 20,
          '#value' => $value->listname,
          '#prefix' => '<div class="listdetails">',
          '#ahah' => array(
              'keypress' => TRUE,
              'path' => "pidlist/ahah/updatename",
              'wrapper' => "allpidlists",
              'method' => 'replace',          
          ),
        );

        $form['alllists'][$key]['changename'] = array(
          '#type' => 'image_button',
          '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/add.png', 
          '#suffix' => '</div>',
          '#ahah' => array(
              'event' => 'click',
              'path' => "pidlist/ahah/updatename",
              'wrapper' => "allpidlists",
              'method' => 'replace',
              ),
        );
        
         $form['alllists'][$key]['labelremove'] = array(
            '#type' => 'item',
            '#title' => t('Delete this list'),
            '#weight' => 20000,
            '#prefix' => '<div class="removelist">',
          );
          
          $form['alllists'][$key]['remove'] = array(
            '#type' => 'image_button',
            '#weight' => 20001,
            '#src' => drupal_get_path('module', 'fedora_repository') . '/images/purge.gif', 
            '#suffix' => '</div>',
            '#ahah' => array(
                'event' => 'click',
                'path' => "pidlist/ahah/remove/list",
                'wrapper' => "allpidlists",
                'method' => 'replace',
                ),
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

        if (user_access("Manage other users' pidlists") || $user->uid == $value->listowner) {
          $fku = array( 
            '#type' => 'fieldset',
            '#collapsible' => TRUE,
            '#collapsed' => FALSE,
            '#title' => t('Users'),
          );

        if (count($value->userlist) != 1) {
          $uindex = array_search($user->uid, $value->userlist, TRUE);
          $fiuser = $value->userlist[0];
          $value->userlist[0] = $value->userlist[$uindex];
          $value->userlist[$uindex] = $fiuser;
        }
        foreach ($value->userlist as $itr => $piduser) {
          if (strlen($piduser) == 0) {
            $fku[$itr]['empty'] = array(
              '#type' => 'item',
              '#value' => t('There are no users associated with this list'),
            );
          }
          else {
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
              'remove' => array(
                '#type' => 'image_button',
                '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png',
                '#ahah' => array(
                  'event' => 'click',
                  'path' => "pidlist/ahah/remove/user",
                  'wrapper' => "allpidlists",
                  'method' => 'replace',
                ),
              ),
              'plo' => array(
                '#type' => 'value',
                '#value' => $value,
              )
            ); 
          }  
        }
        
        $selectoptions = array("" => "- Select A User To Add - ");
        foreach ($options as $index => $item) {
          if ($index == 0) {
            foreach($value->userlist as $val) {
              if (is_null($val)) {
                $selectoptions[$index] = $item['uid'] . ': ' . $item['name'];
              }
            }
          }
          if (!in_array($index, $value->userlist)) {
          $selectoptions[$index] = $item['uid'] . ': ' . $item['name'];
          }  
        }
        
        $fku['listusers'] = array(
          '#type' => 'select',
          '#default value' => t('Select a user to add'),
          '#title' => t('Add a user to the list'),
          '#options' => $selectoptions,
          '#width' => 20,
          '#weight' => 10000,
          '#prefix' => '<div class="addusertolist">',
        );
        $fku['listadd'] = array(
        '#type' => 'image_button',
        '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/add.png', 
        '#suffix' => '</div>',
        '#weight' => 10001, 
        '#ahah' => array(
          'event' => 'click',
          'path' => "pidlist/ahah/add/user",
          'wrapper' => "allpidlists",
          'method' => 'replace',
        ),
      ); 
        
        
      $form['alllists'][$key]['pids'] = array( 
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
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
            'label' => array(
              '#type' => 'item',
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
            ),
            'remove' => array(
              '#type' => 'image_button',
              '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/minus_small.png',
              '#ahah' => array(
                'event' => 'click',
                'path' => "pidlist/ahah/remove/pid",
                'wrapper' => "allpidlists",
                'method' => 'replace',
              ),
            ),
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
          $fkp['pidtext'] = array(
            '#type' => 'textfield',
            '#title' => t('Add a PID'),
            '#size' => 20,
            '#prefix' => '<div class="addpid">',
            '#suffix' => '</div>',
            '#weight' => 10000,
            '#ahah' => array(
              'keypress' => TRUE,
              'path' => "pidlist/ahah/add/pid",
              'wrapper' => "allpidlists",
              'method' => 'replace',          
            ),
          );
          $fkp['add_button'] = array(
         '#type' => 'image_button',
          '#src' => drupal_get_path('module', 'islandora_pid_list') . '/images/add.png', 
          '#weight' => 10001,  
          '#ahah' => array(
          'event' => 'click',
          'path' => "pidlist/ahah/add/pid",
          'wrapper' => 'allpidlists',
          'method' => 'replace',
        ),
      );
          
      }
    }
   }
   else {
    $form['alllists']['empty'] = array(
      '#type' => 'item',
      '#value' => t('No lists currently associated'),
      '#prefix' => '<div class="emptylist">',
      '#suffix' => '</div>',
    );  
   }
 }
  return $form;
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
  $redirectpid = explode("?", $redirectpid);
  $redirectpid = $redirectpid[1];
  
  $options = array("" => "- Select A User To Add - ");
  $query = db_query("SELECT uid, name FROM {users} WHERE uid <> %d ORDER BY uid", $user->uid);
  while ($result = db_fetch_object($query)) {
    $resultname = $result->name;
      if ($result->uid == 0) {
        $resultname = 'Anonymous';
      }
      $options[$result->uid]= $result->uid . ': ' . $resultname;
  }
    
  $form = array(
    '#title' => t('Add'),
      '#tree' => TRUE,
      'listname' => array(
        '#type' => 'textfield',
        '#title' => t('List Name'),
        '#size' => 20,  
      ),
      'listusers' => array(
        '#type' => 'select',
        '#default value' => t('Select a user to add'),
        '#title' => t('Add a user to the list'),
        '#options' => $options,
        '#weight' => 10000,
      ),
      'pidtext' => array(
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
  
  $form['submit'] = array(
      '#type' => 'submit',
      '#name' => 'islandora_pid_list_add_form_submit_button',
      '#value' => t('Submit'),
  );
  $form['clear'] = array(
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

    if ($user->uid == 0) {
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
        drupal_set_message($e->getMessage() . ' The list was still created!', 'error');
      }
    }
    if ($user->uid == 0) {
      $savedlist = 'Note that lists created when not logged will not be saved.';
    }
    drupal_set_message(t('The new list %listname was created. %saved',  
        array(
          '%listname' => $createname,
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
    drupal_goto('pidlist/add', $redirectpid);
  }
 }

  function islandora_pid_list_pid_form($form_state, $pid) {
    global $user;
    $pidlist = array();
    $pidlist = get_user_pid_list(array('pid' => $pid));
    

    $form = array(
      '#title' => t('Lists'),
      '#tree' => TRUE,
      'userlists' => array(
        '#type' => 'fieldset',
        '#title' => t('My Lists'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
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
            '#title' => check_plain($value->listname . ' [ID:' . $idoutput . ']'),  
            '#prefix' => '<div class="userlists">',
        );
        if (user_access("Manage other users' pidlists") || $user->uid == $value->listowner) {
          $form['userlists'][$key]['remove'] = array(
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
      if (!in_array($listid['listid'], $listids)) {
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
      foreach ($_SESSION['islandora_pid_list'] as $value) {
        $sessionlist = unserialize($value);
        $output = str_replace('session_', '', $sessionlist->listid);
          if (!in_array($sessionlist->listid, $listids)) {
            $options[$sessionlist->listid] = $output . ': ' . $sessionlist->listname;
          }
      }
    }
        
    $form['userlists']['listoptions'] = array(
          '#type' => 'select',
          '#default value' => t('Select a list to add the object to'),
          '#options' => $options,
          '#prefix' => '<div class="addlist">',
    );
    
     if (user_access(t('Adminisiter islandora_pid_list module'))) {
      $form['alllists'] = array(
        '#type' => 'fieldset',
        '#title' => t('All Other Lists associated to this PID'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
      );
     foreach ($pidlist as $key => $value) {
      if (!in_array($user->uid, $value->userlist)) {

        $form['alllists'][$key]['label'] = array( 
          '#type' => 'item',
          '#title' => check_plain($value->listname . ' [ID:' . $value->listid . ']'),  
          '#prefix' => '<div class="userlists">',
        );
        $form['alllists'][$key]['remove'] = array(
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

    $form['submit'] = array(
        '#type' => 'submit',
        '#name' => 'islandora_pid_list_pid_form_submit_button',
        '#value' => t('Submit'),
    );
    $form['clear'] = array(
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

function islandora_pid_list_add_pid_js() {
  module_load_include('inc', 'php_lib', 'Ahah');
  dsm($_POST, 'post');
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
  
  if (array_key_exists('userlists', $_POST)) {
    $param = 'userlists';
    $listindex = 0;
    foreach($_POST['userlists'] as $key => $value) {
      if (strlen($value['pids']['pidtext']) != 0) {
        $listindex = $key;
        $foundpidtext = TRUE;
        break;
      }
    }
  }
  
  if (array_key_exists('alllists', $_POST)) {
    $param = 'alllists';
    $listindex = 0;
    if (!$foundpidtext) {
      foreach($_POST['alllists'] as $key => $value) {
        if (strlen($value['pids']['pidtext']) != 0) {
          $listindex = $key;
          break;
        }  
      }
    }
  }
  $pidindex = 0;
    
  if (strlen($_POST[$param][$listindex]['pids']['pidtext']) != 0) {
    foreach($form[$param][$listindex]['pids'] as $item => $assoc) {
      if (is_numeric($item)) {
        $pidindex++;
      }
    }
    $obj = $form[$param][$listindex]['plo']['#value'];
    $new_pid = $_POST[$param][$listindex]['pids']['pidtext'];
      if (!empty($new_pid) && !in_array($new_pid, $obj->pidlist)) {
        try {
          $obj->add_pids(array($new_pid));
        } 
        catch (Exception $e) {
          drupal_set_message($e->getMessage(), 'error');
        }
      }
      else {
      drupal_set_message(t("The pid %pid already exists in %listname",
              array(
                '%pid' => $new_pid,
                '%listname' => $obj->listname,
                )
          ), 'error');    
      }
  }
  else {
    $obj = $form[$param][$listindex]['plo']['#value'];
    drupal_set_message(t("Please enter a pid to add to %listname",
              array(
                '%listname' => $obj->listname,
                )
          ), 'error');  
  }
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  $form[$param][$listindex]['#collapsed'] = FALSE;
  Ahah::respond($form);
  exit();
}

function islandora_pid_list_remove_user_js() {
  global $user;
  // Retrieve the cached form
  module_load_include('inc', 'php_lib', 'Ahah');
  
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
  
  
  $numlists = array(
    'userlists' => count($_POST['userlists']),
    'alllists' => count($_POST['alllists']),
  );
  
  if (array_key_exists('userlists', $_POST)) {
    foreach($_POST['userlists'] as $key => $value) {
      foreach($_POST['userlists'][$key] as $item => $index) {
        if ($item == 'users') {
          foreach ($_POST['userlists'][$key][$item] as $offset => $object) {
            if(!empty($object)) {
              $param = 'userlists';
              $listindex = $key;
              $foundusertext = TRUE;
              $useroffset = $offset;
              break;
            }
          }
        }
      }
    }
  }
  
  if (array_key_exists('alllists', $_POST)) {
    if (!$foundusertext) {
      foreach($_POST['alllists'] as $key => $value) {
        foreach($_POST['alllists'][$key] as $item => $index) {
          if ($item == 'users') {
            foreach ($_POST['alllists'][$key][$item] as $offset => $object) {
              if(!empty($object)) {
                $param = 'alllists';
                $listindex = $key;
                $foundusertext = TRUE;
                $useroffset = $offset;
                break;
              }
            }
          }
        }
      }
    }
  }
  
  $obj = $form[$param][$listindex]['plo']['#value'];
  $obj->remove_users(array($form[$param][$listindex]['users'][$useroffset]['user']['#value']));
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  $form[$param][$listindex]['#collapsed'] = FALSE;
  Ahah::respond($form);
  exit();
}

function islandora_pid_list_remove_pid_js() {
  module_load_include('inc', 'php_lib', 'Ahah');
  // Retrieve the cached form
 list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
    
  if (array_key_exists('userlists', $_POST)) {
    foreach($_POST['userlists'] as $key => $value) {
      foreach($_POST['userlists'][$key] as $item => $index) {
        if ($item == 'pids') {
          foreach ($_POST['userlists'][$key][$item] as $offset => $object) {
            if(!empty($object) && is_array($object)) {
              $param = 'userlists';
              $listindex = $key;
              $foundpidtext = TRUE;
              $pidoffset = $offset;
              break;
            }
          }
        }
      }
    }
  }
  
  if (array_key_exists('alllists', $_POST)) {
    if (!$foundpidtext) {
      foreach($_POST['alllists'] as $key => $value) {
        foreach($_POST['alllists'][$key] as $item => $index) {
          if ($item == 'pids') {
            foreach ($_POST['alllists'][$key][$item] as $offset => $object) {
              if(!empty($object) && is_array($object)) {
                $param = 'alllists';
                $listindex = $key;
                $pidoffset = $offset;
                break;
              }
            }
          }
        }
      }
    }
  }
  $obj = $form[$param][$listindex]['plo']['#value'];
  $obj->remove_pids(array($form[$param][$listindex]['pids'][$pidoffset]['pid']['#value']));
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
   
  $form[$param][$listindex]['#collapsed'] = FALSE;
  Ahah::respond($form);
  exit();
}

function islandora_pid_list_add_user_js() {
  global $user;
  module_load_include('inc', 'php_lib', 'Ahah');
  
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
  
  if (array_key_exists('userlists', $_POST)) {
    foreach($_POST['userlists'] as $key => $value) {
      foreach($_POST['userlists'][$key] as $item => $index) {
        if ($item == 'users') {
          $param = 'userlists';
          $listindex = $key;
          if (is_array($index)) {
            if (array_key_exists('listadd', $index)) {
              $adduser = $index['listusers'];
              $foundselect = TRUE;
              break;
            }  
          }
        }
      } 
    }
  }
      
  if (array_key_exists('alllists', $_POST)) {
    if (!$foundselect) {
      foreach($_POST['alllists'] as $key => $value) {
        foreach($_POST['alllists'][$key] as $item => $index) {
          if ($item == 'users') {
            $param = 'alllists';
            $listindex = $key;
            if (is_array($index)) {
              if (array_key_exists('listadd', $index)) {
                $adduser = $index['listusers'];
                break;
              }  
            }
          }
        } 
      }
    }
  }
  
  $obj = $form[$param][$listindex]['plo']['#value'];
   
  if (strlen($adduser)) {
    $obj->add_users(array($adduser));
  }
  else {
    // "please select a user to add"
    drupal_set_message(t("Please select a user to add to %listname",
              array(
                '%listname' => $obj->listname,
                )
          ), 'error');  
  }
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  $form[$param][$listindex]['#collapsed'] = FALSE;
  Ahah::respond($form);
  exit();
}

function islandora_pid_list_remove_list_js() {
  global $user;
  module_load_include('inc', 'php_lib', 'Ahah');
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
  
  if (array_key_exists('userlists', $_POST)) {
    foreach($_POST['userlists'] as $key => $value) {
      if ($_POST['userlists'][$key]['remove']) {
          $param = 'userlists';
          $foundremove = TRUE;
          $listindex = $key;
          break;
      }
    } 
  }
      
  if (array_key_exists('alllists', $_POST)) {
    if (!$foundremove) {
      foreach($_POST['alllists'] as $key => $value) {
        if ($_POST['alllists'][$key]['remove']) {
          $param = 'alllists';
          $listindex = $key;
          break;
        }  
      }
    }
  }
  
  $obj = $form[$param][$listindex]['plo']['#value'];
  
  if ($param == 'alllists') {
    $obj->remove_list();
  }
  else {
    $obj->remove_users(array($user->uid));
  }
    
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  $form[$param][$listindex]['#collapsed'] = FALSE;
  Ahah::respond($form);
  exit();
}

function islandora_pid_list_update_name_js() {
  module_load_include('inc', 'php_lib', 'Ahah');
  dsm($_POST, 'post');
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
  dsm($form, 'form');
  if (array_key_exists('userlists', $_POST)) {
    foreach($_POST['userlists'] as $key => $value) {
      if ($_POST['userlists'][$key]['changename']) {
          $param = 'userlists';
          $obj = $form[$param][$listindex]['plo']['#value'];
          $newlistname = $_POST[$param][$listindex]['listname'];
          if ($obj->listname != $newlistname) {
            $foundchange = TRUE;
            $listindex = $key;
            break;
          }
      }
    } 
  }
      
  if (array_key_exists('alllists', $_POST)) {
    if (!$foundchange) {
      foreach($_POST['alllists'] as $key => $value) {
        dsm($key, 'key');
        if ($_POST['alllists'][$key]['changename']) {
          $param = 'alllists';
          $obj = $form[$param][$listindex]['plo']['#value'];
          $newlistname = $_POST[$param][$listindex]['listname'];
          if ($obj->listname != $newlistname) {
            $foundchange = TRUE;
            $listindex = $key;
            break;
          }
        }  
      }
    }
  }
  dsm($param, 'param');
  dsm($listindex, 'listindex');
  
  
  dsm($obj, 'obj');
  if ($obj->listname != $newlistname) {
    if (ctype_space($newlistname) || $newlistname == '') {
      drupal_set_message(t('The list %listname must have a name!',
          array(
          '%listname' => $obj->listname,  
          )
      ), 'error');
    }
    else {
      dsm('changethename');
      $obj->change_list_name($newlistname);
    }
  }
   
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  $form[$param][$listindex]['#collapsed'] = FALSE;
  Ahah::respond($form);
  exit();
}