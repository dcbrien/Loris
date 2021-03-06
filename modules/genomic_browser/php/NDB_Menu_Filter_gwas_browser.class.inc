<?php
/**
 * For gwas_browser class file
 * Displays GWAS analysis results 
 * Filterable by SNP and p-value, and other fields.
 *
 * PHP version 5
 *
 *  @category Genomic
 *  @package  Main
 *  @author   Christine Rogers <christine.rogers@mcgill.ca>
 *  @license  http://www.gnu.org/licenses/gpl-3.0.txt @GPLv3
 *  @link     https://www.github.com/aces/Loris/
 *  Main page: CNV. Submenu: GWAS
 */

require_once 'NDB_Menu_Filter.class.inc';
/**
 * NDB_Menu_Filter_GWAS_Browser Class
 *
 * This class is gwas_browser Form
 * GWAS submenu tab for Genomic Browser
 *
 * @category Genomic
 * @package  Main
 * @author   Christine Rogers <christine.rogers@mcgill.ca>
 * @license  http://www.gnu.org/licenses/gpl-3.0.txt @GPLv3
 * @link     https://github.com/aces/Loris
*/
class NDB_Menu_Filter_GWAS_Browser extends NDB_Menu_Filter
{
    /**
     * Variables to enable special filter behaviour -
     * Show brief results or show full results (all available fields)
     * used in (overridden) _addValidFilters function below
     */
    var $_displayBrief = true;  // default: display only Brief results
    var $_briefFields  = array(); // which fields to display

    /**
     * Overloading this method to allow access to site users (own site only)
     * and users w/ multisite privileges
     *
     * @note   overloaded function
     * @return bool
     */
    function _hasAccess()
    {
        // create user object
        $user = User::singleton();
        return ($user->hasPermission('genomic_browser_view_allsites')
                || $user->hasPermission('genomic_browser_view_site'));
    }

    /**
     * Function _setupVariables
     *
     * @note   overloaded function
     * @return bool
    */
    function _setupVariables()
    {
        // set the class variables
        $this->columns = array(
                          'GWAS_JMP_glimmix.rsID AS SNP_ID',
                          'GWAS_JMP_glimmix.Chromosome AS Chromosome',
                          'GWAS_JMP_glimmix.Position_BP AS Position_BP',
                          'GWAS_JMP_glimmix.MajorAllele AS Major_Allele',
                          'GWAS_JMP_glimmix.MinorAllele AS Minor_Allele',
                          'GWAS_JMP_glimmix.MAF AS MAF',
                          'GWAS_JMP_glimmix.Estimate AS Estimate',
                          'GWAS_JMP_glimmix.StdErr AS StdErr',
                          'GWAS_JMP_glimmix.Pvalue AS P_value', 
                         );

        $this->_briefFields = array(
                          'SNP_ID',
                          'Chromosome',
                          'Position_BP',
                          'Major_Allele',
                          'Minor_Allele',
                          'MAF',
                          'Estimate',
                          'StdErr',
                          'P_value', 
                         );

        $this->query = " FROM GWAS_JMP_glimmix "; 

        $this->order_by     = 'GWAS_JMP_glimmix.Chromosome, GWAS_JMP_glimmix.Pvalue DESC';
        $this->validFilters = array(
                               'GWAS_JMP_glimmix.rsID',
                               'GWAS_JMP_glimmix.Chromosome',
                               'GWAS_JMP_glimmix.Position_BP',
                               'GWAS_JMP_glimmix.MajorAllele',
                               'GWAS_JMP_glimmix.MinorAllele',
                               'GWAS_JMP_glimmix.MAF',
                               'GWAS_JMP_glimmix.Estimate',
                               'GWAS_JMP_glimmix.StdErr',
                               'GWAS_JMP_glimmix.Pvalue',
                              );

        $this->formToFilter = array(
                               'SNP_ID'       => 'GWAS_JMP_glimmix.rsID',
                               'Chromosome'   => 'GWAS_JMP_glimmix.Chromosome',
                               'Position_BP'  => 'GWAS_JMP_glimmix.Position_BP',
                               'Major_Allele' => 'GWAS_JMP_glimmix.Major_Allele',
                               'Minor_Allele' => 'GWAS_JMP_glimmix.Minor_Allele',
                               'MAF'          => 'GWAS_JMP_glimmix.MAF',
                               'Estimate'     => 'GWAS_JMP_glimmix.Estimate',
                               'StdErr'       => 'GWAS_JMP_glimmix.StdErr',
                               'P_value'      => 'GWAS_JMP_glimmix.P_value', 
                              );

        return true;
    }

    /**
     * Sets the template data for the filter form
     *
     * @note   overloaded function
     * @return bool
     */
    function _setFilterForm()
    {
        // create user object
        $user = User::singleton();

        // add form elements
        $show_results_options
            = array(
               'brief' => 'Summary fields',
               'full'  => 'All fields',
              );
        $this->addSelect('Show_Brief_Results', 'Display:', $show_results_options);

        $this->addBasicText('SNP_ID', 'SNP ID:');
        $this->addBasicText('Chromosome', 'Chromosome:');
        $this->addBasicText('BP_Position', 'BP Position:');
        $this->addBasicText('MAF', 'MAF:');

        $alleleArray = array(
             ''  => 'Any',
             'A' => 'A',
             'C' => 'C',
             'T' => 'T',
             'G' => 'G',
            );
        $this->addSelect('Minor_Allele', 'Minor Allele:', $alleleArray); 
        $this->addSelect('Major_Allele', 'Major Allele:', $alleleArray); 

        $this->addBasicText('Estimate', 'Estimate:');
        $this->addBasicText('StdErr', 'Std Err:');
        $this->addBasicText('Pvalue', 'P-value:');

        return true;
    }

    /**
     * Function _setDataTableRows
     *
     * @param string $count number of data
     *
     * @note   overloaded function
     * @return bool
     */
    function _setDataTableRows($count)
    {
        // create user object
        $user           = User::singleton();
        $subprojectlist = Utility::getSubprojectList();

        $this->tpl_data['resultcount']  = $this->TotalItems;
        $this->tpl_data['displayBrief'] = $this->_displayBrief;

        // re-set headers if Brief; if full, gets set by _setDataTable()
        if ($this->_displayBrief) {
            $this->headers = array(); // re-initialize i.e. wipe
            foreach ($this->_briefFields as $col) {
                $this->headers[] = $col;
            }

            // copy column headings to new array, then replace
            $colCount     = 0; // column counter
            $maxCols      = sizeof($this->tpl_data['headers']); //limit
            $briefHeaders = array();  // reset
            foreach ($this->headers as $header) {
                $found = false;
                while ($colCount < $maxCols && !$found ) {
                    // copy entire tpl_data element
                    // including displayName and fieldOrder
                    if ($this->tpl_data['headers'][$colCount]['name'] == $header) {
                        $found          = true;
                        $briefHeaders[] = $this->tpl_data['headers'][$colCount];
                    }
                    $colCount++;
                } // iterate to check for next elt, starting from here
            }
            // When done, replace tpl_data headers
            $this->tpl_data['headers'] = $briefHeaders;
        }

        $x = 0;
        foreach ($this->list as $item) {
            //count column
            $this->tpl_data['items'][$x][0]['value'] = $x + $count;

            //print out data rows
            $i = 1;
            foreach ($item as $key => $val) {

                if ($this->_displayBrief && !in_array($key, $this->_briefFields)) {
                    continue;  // no increment to $i
                }

                //Show the URL to the timepoint list on PSCID field
                if ($key == 'SNP_ID' && $user->hasPermission('access_all_profiles')) {
                    $this->tpl_data['items'][$x][$i]['SNP_ID'] = $item['SNP_ID'];
                }
                $this->tpl_data['items'][$x][$i]['name']  = $key;
                $this->tpl_data['items'][$x][$i]['value'] = $val;

                $i++;
            }
            $x++;
        }
        $this->tpl_data['resultcount']   = $this->TotalItems;
        $this->tpl_data['displayFilter'] = $this->_displayBrief;

        return true;
    }

    /**
     * Adds filters
     * This function overrides filters to enable display of brief/full results
     *
     * @param string $prepared_key filter key
     * @param string $field        filter field
     * @param string $val          filter value
     *
     * @note overloaded function
     *
     * @return $query
    */
    function _addValidFilters($prepared_key, $field, $val)
    {
        $query = ''; //initialize
        if ($field == "show_brief_results") {
            if ($val == "full") {
                $this->_displayBrief = false;
            }
            return $query;
        }

        if ((!empty($val) || $val === '0') && $field != 'order') {
            if (in_array($field, $this->CheckboxFilters) && $val) {
                 $query .= " AND $field";
            } elseif (strtolower(substr($field, -8)) == 'centerid'
                || strtolower(substr($field, -10)) == 'categoryid'
                || strtolower(substr($field, -6)) == 'gender'
                || strtolower(substr($field, -10)) == 'chromosome'
                || (isset($this->EqualityFilters)
                && in_array($field, $this->EqualityFilters))
            ) {

                 $query .= " AND $field = :v_$prepared_key";
            } else {
                 $query .= " AND $field LIKE CONCAT('%', :v_$prepared_key, '%') ";
            }
        }
        return $query;
    }

}
?>
