sprout_add_test(sprout_unit_reflection_typeregistry
    SOURCES
        TypeRegistryTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/reflection/TypeRegistry.cpp
    LABELS unit
)

target_include_directories(sprout_unit_reflection_typeregistry PRIVATE
    "${SPROUT_ROOT_DIR}/engine/src"
)

sprout_add_test(sprout_unit_reflection_propertyvalue
    SOURCES
        PropertyValueTests.cpp
    LABELS unit
)

target_include_directories(sprout_unit_reflection_propertyvalue PRIVATE
    "${SPROUT_ROOT_DIR}/engine/include"
)

sprout_add_test(sprout_unit_reflection_attributes
    SOURCES
        AttributesTests.cpp
    LABELS unit
)

target_include_directories(sprout_unit_reflection_attributes PRIVATE
    "${SPROUT_ROOT_DIR}/engine/include"
)