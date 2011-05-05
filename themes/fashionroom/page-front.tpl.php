<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="<?php print $language->language; ?>" lang="<?php print $language->language; ?>" dir="<?php print $language->dir; ?>">

<head>
  <title><?php print $head_title; ?></title>
  <?php print $head; ?>
  <?php print $styles; ?>
  <?php print $scripts; ?>

<!--[if lte IE 6]>
<style>
.block, .whiteblock, .dialog {
    position: absolute;
}
</style>
<![endif]--></head>
    <body class="wk ">
       
       
        <div id="progress" style="display: none; ">
        </div>
        <div id="polyvore_css_load_test" style="position:absolute; width: 0px; height: 0px; left: -1000px">
        </div>
        <div id="body" style="min-height: 610px; ">
            <div id="stripe">
            </div>
            <div id="header" class="has_search">
                <table cellpadding="0" cellspacing="0" border="0" id="nav" width="99%">
                    <tbody><tr>
            <td valign="top" width="315"><?php if ($logo): ?>
        <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home" id="logo"><img src="<?php print $logo; ?>" alt="<?php print t('Home'); ?>" /></a>
      <?php endif; ?></td>
            <td valign="top" style="white-space:nowrap">
                <?php if ($primary_links): ?>
    <div id="skip-link"><a href="#main-menu"><?php print t('Jump to Navigation'); ?></a></div>
  <?php endif; ?>
            </td>
            <td align="right" width="1%" id="header_search">
                <?php if ($search_box): ?>
        <div id="search-box"><?php print $search_box; ?></div>
      <?php endif; ?>
            </td>
        </tr>
    </tbody>
                </table>
            </div>
            <div class="page" id="shop_page">
                <div id="left">
                    <div class="box categories">
                       <?php print $content ?>
                    </div>
                </div>
              <div id="right">
                    
               <?php if ($right): ?>
        <div id="sidebar-right" class="sidebar">
          <?php if (!$left && $search_box): ?><div class="block block-theme"><?php print $search_box ?></div><?php endif; ?>
          <?php print $right ?>
        </div>
      <?php endif; ?>    
              </div>
                <br class="clear">
              <div class="box">
                
                </div>
                <br class="clear">
            </div>
    <?php print $closure; ?>        
    </body>
</html>
