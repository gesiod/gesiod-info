<?php
// $Id: views_handler_field_taxonomy_image_views_options.inc,v 1.1 2011/02/09 21:01:48 scyther Exp $

/**
 * @file
 * Views integration for Taxonomy Image Views Options module.
 */

/**
 * Field handler to provide an embedded image.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_taxonomy_image_views_options extends views_handler_field {
  /**
   * Define options available for this field.
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['display_format'] = array('default' => 'image_plain');
    return $options;
  }

  /**
   * Build option configuration form.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $presets = array(
      'image_plain' => t('Image'),
      'image_link_taxonomy' => t('Image linked to term page'),
      'image_link_image' => t('Image linked to image'),
      'path_to_image' => t('Path to image'),
      'url_to_image' => t('URL to image'),
    );

    if (module_exists('imagecache')) {
      foreach (imagecache_presets() as $preset) {
        $presets['imagecache_' . $preset['presetname'] . '_default'] = t('@preset image', array('@preset' => $preset['presetname']));
        $presets['imagecache_' . $preset['presetname'] . '_termlink'] = t('@preset image linked to term page', array('@preset' => $preset['presetname']));
        $presets['imagecache_' . $preset['presetname'] . '_imagelink'] = t('@preset image linked to image', array('@preset' => $preset['presetname']));
        $presets['imagecache_' . $preset['presetname'] . '_path'] = t('@preset file path', array('@preset' => $preset['presetname']));
        $presets['imagecache_' . $preset['presetname'] . '_url'] = t('@preset URL', array('@preset' => $preset['presetname']));
      }
    }

    $form['display_format'] = array(
      '#type' => 'select',
      '#title' => t('Display'),
      '#options' => $presets,
      '#default_value' => $this->options['display_format'],
    );
  }

  /**
   * Render field output to the browser.
   */
  function render($values) {
    $display_format = $this->options['display_format'];
    $tid = $values->{$this->field_alias};

    if (!$tid) {
      return;
    }

    if (drupal_substr($display_format, 0, 11) == 'imagecache_') {
      $preset = drupal_substr($display_format, 11);
      $preset = drupal_substr($preset, 0, strrpos($preset, '_'));

      return theme('taxonomy_image_views_options_formatter_' . $display_format, $tid, $preset);
    }
    else {
      return theme('taxonomy_image_views_options_formatter_' . $display_format, $tid);
    }
  }
}
