create or replace
TRIGGER T_UPDATE_PAGE_IMAGE_VIEW 
INSTEAD OF UPDATE ON PAGE_IMAGE_VIEW 

-- THIS CODE IS (and should be) VERY SIMILAR TO T_INSERT_PAGE_IMAGE_VIEW.

declare
--some temporary variables we will use in the trigger.
imageID varchar2(2000);
imageDatasetID varchar2(2000);
pepvDatasetID varchar2(2000);

pageElementID varchar(2000);
pageElementDatasetID varchar2(2000);
assoPageElementDatasetID varchar2(2000);

pageElementValueID varchar2(2000);

assoPEPVDatasetID varchar2(2000);
encoding varchar2(2000);
insert_value clob;

image_dataset_list dataset_list_t;
image_dataset dataset_t;

page_element_list page_element_list_t;
page_element page_element_t;

page_element_dataset_list dataset_list_t;
page_element_dataset dataset_t;

asso_page_element_dataset_list associating_dataset_list_t;
asso_page_element_dataset associating_dataset_t;

page_element_value value_t;
pepv_dataset_list dataset_list_t;
pepv_dataset dataset_t;
asso_pepv_dataset associating_dataset_t;

property property_t;
propertyID varchar2(2000);

value_type type_t;
valueTypeID varchar2(2000);

contributor_list contributor_list_t;
contributor contributor_t;

image_type_list type_list_t;

longString clob;

copyright_list copyright_list_t;
v_count number;

boundary_list boundary_list_t;
boundary boundary_t;

BEGIN
  imageID := :new.id;
   
  --manage the type information of each page image
  image_type_list := :new.type_list;
  if(image_type_list is not null and image_type_list.last>0) then
    for i in image_type_list.first .. image_type_list.last loop
      v_count := 0;
      SELECT count(*) INTO v_count FROM datatype WHERE id = image_type_list(i).id;
      if(v_count = 0) then
        insert into datatype (id, name, description) 
          values (image_type_list(i).id, image_type_list(i).name, image_type_list(i).description);
      else
        if(image_type_list(i).name is not null) then
          update datatype set name=image_type_list(i).name where id=image_type_list(i).id;
        end if;
        if(image_type_list(i).description is not null) then
          update datatype set description=image_type_list(i).description where id=image_type_list(i).id;
        end if;
      end if;
      SELECT count(*) INTO v_count FROM associate_datatype_data_item WHERE datatype_id = image_type_list(i).id and data_item_id = imageID; 
      if(v_count = 0) then
        insert into associate_datatype_data_item (data_item_id, datatype_id) 
          values (imageID, image_type_list(i).id);
      end if;
    end loop;
  end if;
  
  --manage the copyright information of each page image 
  copyright_list := :new.copyright_list;
  if(copyright_list is not null and copyright_list.last>0) then
    for i in copyright_list.first .. copyright_list.last loop
      insert into has_copyright (data_item_id, file_id) 
        values (imageID, copyright_list(i).id);
    end loop;
  end if;
  
  --manage the relationships between a page image and the datasets that include it
  if(:new.image_dataset_list is not null and :new.image_dataset_list.last > 0) then
    --store the current image_dataset_list into a variable for further references
    image_dataset_list := :new.image_dataset_list;
    for i in image_dataset_list.first .. image_dataset_list.last loop
      image_dataset := image_dataset_list(i);
      imageDatasetID := image_dataset.id;
      
      --we have got the dataset ID that includes the current page image; then we insert a record describing their relationship
      v_count := 0;
      SELECT count(*) INTO v_count FROM includes_page_image WHERE dataset_id = imageDatasetID AND page_image_id = imageID;
      if(v_count = 0) then
        insert into includes_page_image (dataset_id, page_image_id) 
          values (imageDatasetID, imageID);
      end if;
    end loop; --the end of going through all datasets
  end if; --the end of managing page image and its datasets
  
  --start to manage the page element information related to this page image
  page_element_list := :new.page_element_list;
  if(page_element_list is not null and page_element_list.last>0) then
    for j in page_element_list.first .. page_element_list.last loop
      page_element := page_element_list(j);
      pageElementID := page_element.id;
      insert into data_item (id, description, flag) values (pageElementID, page_element.description, 'page_element');
      insert into image_data_item (id) values (pageElementID);
      insert into physical_image_data_item (id) values (pageElementID);
      
      --manage the boundaries of this page element
      boundary_list := page_element.boundary_list;
      --if(boundary_list is not null and boundary_list.last > 0) then        
        --for k in boundary_list.first .. boundary_list.last loop
          --boundary := boundary_list(k);
          
          --insert each particular boundary into the sub-table of boundary
          --insert into table(select p.boundary from page_element p where p.id=pageElementID) b
          --(type, description, value) values (boundary.type, boundary.description, boundary.value);
        --end loop;
      --end if; --the end of managing page element and its datasets
      
      insert into page_element (id, topleftx, toplefty, width, height, number_of_pixels, boundary, mask) 
        values (pageElementID, page_element.topleftx, page_element.toplefty, page_element.width, page_element.height, 
                page_element.number_of_pixels, boundary_list, page_element.mask);
      
      --insert a record describing that the current page image contains this page element
      insert into contains_page_element (page_image_id, page_element_id) 
        values (imageID, pageElementID);
      
      --manage the type information of each page element
      if(page_element.type_list is not null and page_element.type_list.last>0) then
        for i in page_element.type_list.first .. page_element.type_list.last loop
          v_count := 0;
          SELECT count(*) INTO v_count FROM datatype WHERE id = page_element.type_list(i).id;
          if(v_count = 0) then
            insert into datatype (id, name, description) 
              values (page_element.type_list(i).id, page_element.type_list(i).name, page_element.type_list(i).description);
          else
            if(page_element.type_list(i).name is not null) then
              update datatype set name=page_element.type_list(i).name where id=page_element.type_list(i).id;
            end if;
            if(page_element.type_list(i).description is not null) then
              update datatype set description=page_element.type_list(i).description where id=page_element.type_list(i).id;
            end if;
          end if;
          insert into associate_datatype_data_item (data_item_id, datatype_id) 
            values (pageElementID, page_element.type_list(i).id);
        end loop;
      end if;
      
      --manage the copyright information of each page element
      copyright_list := page_element.copyright_list;
      if(copyright_list is not null and copyright_list.last>0) then
        for i in copyright_list.first .. copyright_list.last loop
          insert into has_copyright (data_item_id, file_id) 
            values (pageElementID, copyright_list(i).id);
        end loop;
      end if;
      
      --manage this page element's contributor list
      contributor_list := page_element.contributor_list;
      if(contributor_list is not null and contributor_list.last>0) then
        for i in contributor_list.first .. contributor_list.last loop
          contributor := contributor_list(i);
          insert into contribute (person_id, data_item_id, description, type) 
            values (contributor.person_id, pageElementID, contributor.description, contributor.type);
        end loop; --the end of going through all datasets
      end if; --the end of managing page image's contributor list
      
      --manage the datasets that include this page element
      page_element_dataset_list := page_element.dataset_list;
      if(page_element_dataset_list is not null and page_element_dataset_list.last > 0) then        
        for k in page_element_dataset_list.first .. page_element_dataset_list.last loop
          page_element_dataset := page_element_dataset_list(k);
          pageElementDatasetID := page_element_dataset.id;
          
          --insert a record describing their relationship
          insert into includes_page_element (dataset_id, page_element_id) 
            values (pageElementDatasetID, pageElementID);              
        end loop;
      end if; --the end of managing page element and its datasets
      
      --manage this page element's property values
      if(page_element.value_list is not null and page_element.value_list.last > 0) then
        for k in page_element.value_list.first .. page_element.value_list.last loop
          page_element_value := page_element.value_list(k);
          pageElementValueID := page_element_value.id;

          insert into data_item (id, description, flag) values (pageElementValueID, page_element_value.description, 'page_element_property_value');
          insert into image_data_item (id) values (pageElementValueID);
          insert into logical_image_data_item (id) values (pageElementValueID);

          insert_value := page_element_value.value;
          if(page_element_value.encoding is not null) then
            encoding := page_element_value.encoding;
            if(encoding = 'base64') then
              insert_value := to_clob(utl_raw.cast_to_varchar2(UTL_ENCODE.BASE64_DECODE(utl_raw.cast_to_raw(dbms_lob.substr(insert_value)))));
            end if;
          end if;
          
          insert into page_element_property_value (id, value) values (pageElementValueID, insert_value);
          
          --insert a record describing that the current page element has this page element value
          insert into has_value (page_element_id, page_element_property_value_id) 
            values (pageElementID, pageElementValueID);

           --manage the copyright information of each page element property value
          copyright_list := page_element_value.copyright_list;
          if(copyright_list is not null and copyright_list.last>0) then
            for i in copyright_list.first .. copyright_list.last loop
              insert into has_copyright (data_item_id, file_id) 
                values (pageElementValueID, copyright_list(i).id);
            end loop;
          end if;
          
          --manage this page element property value's contributor list
          contributor_list := page_element_value.contributor_list;
          if(contributor_list is not null and contributor_list.last>0) then
            for i in contributor_list.first .. contributor_list.last loop
              contributor := contributor_list(i);
              insert into contribute (person_id, data_item_id, description, type) 
                values (contributor.person_id, pageElementValueID, contributor.description, contributor.type);
            end loop; --the end of going through all datasets
          end if; --the end of managing page image's contributor list
          
          --manage the type information of each page element property value
          if(page_element_value.type_list is not null and page_element_value.type_list.last>0) then
            for i in page_element_value.type_list.first .. page_element_value.type_list.last loop
              v_count := 0;
              SELECT count(*) INTO v_count FROM datatype WHERE id = page_element_value.type_list(i).id;
              if(v_count = 0) then
                insert into datatype (id, name, description) 
                  values (page_element_value.type_list(i).id, 
                  page_element_value.type_list(i).name, page_element_value.type_list(i).description);
              else
                if(page_element_value.type_list(i).name is not null) then
                  update datatype set name=page_element_value.type_list(i).name where id=page_element_value.type_list(i).id;
                end if;
                if(page_element_value.type_list(i).description is not null) then
                  update datatype set description=page_element_value.type_list(i).description where id=page_element_value.type_list(i).id;
                end if;
              end if;
              insert into associate_datatype_data_item (data_item_id, datatype_id) 
                values (pageElementValueID, page_element_value.type_list(i).id);
            end loop;
          end if;
          
          --for this page element property value, we need to manage its dataset list
          if(page_element_value.dataset_list is not null and page_element_value.dataset_list.last > 0) then
            pepv_dataset_list := page_element_value.dataset_list;
            for m in pepv_dataset_list.first .. pepv_dataset_list.last loop
              pepv_dataset := pepv_dataset_list(m);
              pepvDatasetID := pepv_dataset.id;
                      
              --till here, we have got the dataset ID that includes the current pe_pv; then, we insert a record describing their relationship
              insert into includes_pe_pv (dataset_id, page_element_property_value_id) 
                values (pepvDatasetID, pageElementValueID);
            end loop;
          end if; --the end of managing page element property value and its datasets
          
          --manage the property of a pe_pv
          if(page_element_value.property is not null) then
            property := page_element_value.property;
            propertyID := property.id;
            
            v_count := 0;
            SELECT count(*) INTO v_count FROM datatype_property WHERE id = propertyID;
            
            if(v_count = 0) then
              insert into datatype_property (id, name, description) 
                values (propertyID, property.name, property.description);
            else
              if(property.name is not null) then
                update datatype_property set name = property.name where id = propertyID;
              end if;
              if(property.description is not null) then
                update datatype_property set description = property.description where id = propertyID;
              end if;
            end if;

            --insert a record describing that the current property value is the value of this property
            insert into value_of (page_element_property_value_id, data_type_property_id) 
              values (pageElementValueID, propertyID);
            
            --manage the relationship between property and datatypes
            if(property.type_list is not null and property.type_list.last > 0) then
              for p in property.type_list.first .. property.type_list.last loop
                value_type := property.type_list(p);
                valueTypeID := value_type.id;
                
                v_count := 0;
                SELECT count(*) INTO v_count FROM datatype WHERE id = valueTypeID;
                if(v_count = 0) then
                  insert into datatype (id, name, description) 
                    values (valueTypeID, value_type.name, value_type.description);
                else
                  if(value_type.name is not null) then
                    update datatype set name=value_type.name where id=valueTypeID;
                  end if;
                  if(value_type.description is not null) then
                    update datatype set description=value_type.description where id=valueTypeID;
                  end if;
                end if;

                --associate the property to its each type
                insert into has_property (data_type_id, data_property_ID) 
                  values (valueTypeID, propertyID) LOG ERRORS INTO errorlog ('ORA_ERR_MESG$') REJECT LIMIT UNLIMITED;
              end loop;
            end if;
          end if;

        end loop;
      end if;

    end loop;
  end if; --the end of managing page_element_list
  END;