/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PALUDIS_GUARD_PALUDIS_NAME_HH
#define PALUDIS_GUARD_PALUDIS_NAME_HH 1

#include <ostream>
#include <string>
#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/smart_record.hh>
#include <paludis/util/collection.hh>
#include <paludis/util/validated.hh>

namespace paludis
{
    /**
     * A PackageNamePartError is thrown if an invalid value is assigned to
     * a PackageNamePart.
     *
     * \ingroup Database
     * \ingroup Exception
     */
    class PackageNamePartError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            PackageNamePartError(const std::string & name) throw ();
    };

    /**
     * A PackageNamePartValidator handles validation rules for the value
     * of a PackageNamePart.
     *
     * \ingroup Database
     */
    struct PackageNamePartValidator :
        private InstantiationPolicy<PackageNamePartValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a PackageNamePart,
         * throw a PackageNamePartError.
         */
        static void validate(const std::string &);
    };

    /**
     * A PackageNamePart holds a std::string that is a valid name for the
     * category part of a QualifiedPackageName.
     *
     * \ingroup Database
     */
    typedef Validated<std::string, PackageNamePartValidator> PackageNamePart;

    /**
     * Holds a set of PackageNamePart instances.
     *
     * \ingroup Database
     */
    typedef SortedCollection<PackageNamePart> PackageNamePartCollection;

    /**
     * A CategoryNamePartError is thrown if an invalid value is assigned to
     * a CategoryNamePart.
     *
     * \ingroup Exception
     * \ingroup Database
     */
    class CategoryNamePartError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            CategoryNamePartError(const std::string & name) throw ();
    };

    /**
     * A CategoryNamePartValidator handles validation rules for the value
     * of a CategoryNamePart.
     *
     * \ingroup Database
     */
    struct CategoryNamePartValidator :
        private InstantiationPolicy<CategoryNamePartValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a CategoryNamePart,
         * throw a CategoryNamePartError.
         */
        static void validate(const std::string &);
    };

    /**
     * A CategoryNamePart holds a std::string that is a valid name for the
     * category part of a QualifiedPackageName.
     *
     * \ingroup Database
     */
    typedef Validated<std::string, CategoryNamePartValidator> CategoryNamePart;

    /**
     * Holds a set of CategoryNamePart instances.
     *
     * \ingroup Database
     */
    typedef SortedCollection<CategoryNamePart> CategoryNamePartCollection;

    /**
     * Keys for a QualifiedPackageName.
     */
    enum QualifiedPackageNameKeys
    {
        qpn_category,         ///< The CategoryNamePart part
        qpn_package,          ///< The PackageNamePart part
        last_qpn              ///< Number of values
    };

    /**
     * Tags for a QualifiedPackageName.
     */
    struct QualifiedPackageNameTag :
        SmartRecordTag<comparison_mode::FullComparisonTag, comparison_method::SmartRecordCompareByAllTag>,
        SmartRecordKeys<QualifiedPackageNameKeys, last_qpn>,
        SmartRecordKey<qpn_category, CategoryNamePart>,
        SmartRecordKey<qpn_package, PackageNamePart>
    {
    };

    /**
     * A QualifiedPackageName instance holds a CategoryNamePart and
     * a PackageNamePart.
     */
    class QualifiedPackageName :
        public MakeSmartRecord<QualifiedPackageNameTag>::Type
    {
        private:
            static MakeSmartRecord<QualifiedPackageNameTag>::Type _make_parent(
                    const std::string & s);

        public:
            /**
             * Constructor.
             */
            QualifiedPackageName(const CategoryNamePart & c, const PackageNamePart & p) :
                MakeSmartRecord<QualifiedPackageNameTag>::Type(c, p)
            {
            }

            /**
             * Copy constructor.
             */
            QualifiedPackageName(const QualifiedPackageName & other) :
                MakeSmartRecord<QualifiedPackageNameTag>::Type(other)
            {
            }

            /**
             * Constructor, from a raw string.
             */
            explicit QualifiedPackageName(const std::string & s) :
                MakeSmartRecord<QualifiedPackageNameTag>::Type(_make_parent(s))
            {
            }

            /**
             * Assignment.
             */
            const QualifiedPackageName & operator= (const QualifiedPackageName & other)
            {
                MakeSmartRecord<QualifiedPackageNameTag>::Type::operator= (other);
                return *this;
            }
    };

    /**
     * Output a QualifiedPackageName to a stream.
     */
    std::ostream & operator<< (std::ostream &, const QualifiedPackageName &);

    /**
     * Holds a collection of QualifiedPackageName instances.
     */
    typedef SortedCollection<QualifiedPackageName> QualifiedPackageNameCollection;

    /**
     * A QualifiedPackageNameError may be thrown if an invalid name is
     * assigned to a QualifiedPackageName (alternatively, the exception
     * raised may be a PackageNamePartError or a CategoryNamePartError).
     */
    class QualifiedPackageNameError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            QualifiedPackageNameError(const std::string &) throw ();
    };

    /**
     * Convenience operator to make a QualifiedPackageName from a
     * PackageNamePart and a CategoryNamePart.
     */
    inline const QualifiedPackageName
    operator+ (const CategoryNamePart & c, const PackageNamePart & p)
    {
        return QualifiedPackageName(c, p);
    }

    /**
     * A UseFlagNameError is thrown if an invalid value is assigned to
     * a UseFlagName.
     *
     * \ingroup Database
     * \ingroup Exception
     */
    class UseFlagNameError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            UseFlagNameError(const std::string & name) throw ();
    };

    /**
     * A UseFlagNameValidator handles validation rules for the value of a
     * UseFlagName.
     *
     * \ingroup Database
     */
    struct UseFlagNameValidator :
        private InstantiationPolicy<UseFlagNameValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a UseFlagName,
         * throw a UseFlagNameError.
         */
        static void validate(const std::string &);
    };

    /**
     * A UseFlagName holds a std::string that is a valid name for a USE flag.
     *
     * \ingroup Database
     */
    typedef Validated<std::string, UseFlagNameValidator> UseFlagName;

    /**
     * A SlotNameError is thrown if an invalid value is assigned to
     * a SlotName.
     *
     * \ingroup Database
     * \ingroup Exception
     */
    class SlotNameError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            SlotNameError(const std::string & name) throw ();
    };

    /**
     * A SlotNameValidator handles validation rules for the value of a
     * SlotName.
     *
     * \ingroup Database
     */
    struct SlotNameValidator :
        private InstantiationPolicy<SlotNameValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a SlotName,
         * throw a SlotNameError.
         */
        static void validate(const std::string &);
    };

    /**
     * A SlotName holds a std::string that is a valid name for a SLOT.
     *
     * \ingroup Database
     */
    typedef Validated<std::string, SlotNameValidator> SlotName;

    /**
     * A RepositoryNameError is thrown if an invalid value is assigned to
     * a RepositoryName.
     *
     * \ingroup Exception
     * \ingroup Database
     */
    class RepositoryNameError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            RepositoryNameError(const std::string & name) throw ();
    };

    /**
     * A RepositoryNameValidator handles validation rules for the value
     * of a RepositoryName.
     *
     * \ingroup Database
     */
    struct RepositoryNameValidator :
        private InstantiationPolicy<RepositoryNameValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a RepositoryName,
         * throw a RepositoryNameError.
         */
        static void validate(const std::string &);
    };

    /**
     * A RepositoryNamePart holds a std::string that is a valid name for a
     * Repository.
     *
     * \ingroup Database
     */
    typedef Validated<std::string, RepositoryNameValidator> RepositoryName;

    /**
     * Holds a collection of RepositoryName instances.
     */
    typedef SequentialCollection<RepositoryName> RepositoryNameCollection;

    /**
     * A KeywordNameValidator handles validation rules for the value of a
     * UseFlagName.
     *
     * \ingroup Environment
     */
    struct KeywordNameValidator :
        private InstantiationPolicy<KeywordNameValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a KeywordName,
         * throw a KeywordNameError.
         */
        static void validate(const std::string &);
    };

    /**
     * A KeywordNameError is thrown if an invalid value is assigned to
     * a KeywordNameName.
     *
     * \ingroup Environment
     * \ingroup Exception
     */
    class KeywordNameError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            KeywordNameError(const std::string & name) throw ();
    };

    /**
     * A KeywordName holds a std::string that is a valid name for a KEYWORD.
     *
     * \ingroup Database
     */
    typedef Validated<std::string, KeywordNameValidator> KeywordName;

    /**
     * A USE flag can be on, off or unspecified.
     */
    enum UseFlagState
    {
        use_unspecified,     /// unspecified
        use_disabled,        /// disabled
        use_enabled          /// enabled
    };
}

#endif
