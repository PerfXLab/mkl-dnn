/*******************************************************************************
* Copyright 2016 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#ifndef PRIMITIVE_HPP
#define PRIMITIVE_HPP

#include <assert.h>

#include "mkldnn.h"

#include "c_types_map.hpp"
#include "event.hpp"
#include "nstl.hpp"
#include "primitive_desc.hpp"

/** \brief A pure virtual primitive class
 *
 * Primitive contains links to its inputs & outputs, though it does not track
 * their readiness on execution step.
 *
 * @remark @b Rational.
 *   Dependencies are essential through-out the whole MKL-DNN library, so it
 *   makes sense to include them on the very low level. On the other hand,
 *   tracking them should be a task for corresponding essence, like scheduler,
 *   stream or whatever. Primitive itself should know nothing about the
 *   environment it is running in.
 *
 * @note
 *   To make user experience better we should provide API which allows
 *   achieving the best (or good enough) performance when creating primitives
 *   in natural order: i.e. from bottom to top for forward pass and from top to
 *   bottom for backward pass. Please consider restriction [1] in Level 0.
 */
struct mkldnn_primitive: public mkldnn::impl::c_compatible {
    typedef mkldnn::impl::nstl::vector<mkldnn::impl::primitive_at_t>
        input_vector;
    typedef mkldnn::impl::nstl::vector<const mkldnn::impl::primitive_t *>
        output_vector;

    mkldnn_primitive(const mkldnn::impl::primitive_desc_t *pd,
            const input_vector &inputs, const output_vector &outputs)
        : pd_(pd)
        , inputs_(inputs)
        , outputs_(outputs)
    {}
    virtual ~mkldnn_primitive() {}

    /** returns primitive's engine */
    mkldnn::impl::engine_t *engine() const { return pd_->engine(); }
    /** returns primitive's inputs */
    const input_vector &inputs() const { return inputs_; }
    /** returns primitive's outputs */
    const output_vector &outputs() const { return outputs_; }
    /** returns primitive's primitive desc */
    const mkldnn::impl::primitive_desc_t *pd() const { return pd_; }
    /** returns primitive's kind */
    mkldnn::impl::primitive_kind_t kind() const { return pd_->kind(); }

    /** executes primitive with resulting event @p e
     *
     * @p e (output)
     *   a resulting event. It is primitive responsibility to set @p e state
     *   after actual computations are done
     *
     * @remark @b Rational.
     *   Suppose engine has a task pool and for some reasons submission failed.
     *   In this case primitive will set @p e's state to event::error
     */
    virtual void execute(mkldnn::impl::event_t *e) = 0;

    /** returns data handle. Applicable for memory primitives only. */
    virtual mkldnn::impl::status_t get_data_handle(void **handle) const {
        UNUSED(handle);
        assert(this->kind() == mkldnn::impl::primitive_kind::memory);
        return mkldnn::impl::status::invalid_arguments;
    }
    /** sets data handle. Applicable for memory primitives only. */
    virtual mkldnn::impl::status_t set_data_handle(void *handle) {
        UNUSED(handle);
        assert(this->kind() == mkldnn::impl::primitive_kind::memory);
        return mkldnn::impl::status::invalid_arguments;
    }

protected:
    const mkldnn::impl::primitive_desc_t *pd_;
    input_vector inputs_;
    output_vector outputs_;

private:
    mkldnn_primitive() = delete;
    mkldnn_primitive(const mkldnn_primitive &) = delete;
    mkldnn_primitive(mkldnn_primitive &&) = delete;
    mkldnn_primitive &operator=(const mkldnn_primitive &) = delete;
    mkldnn_primitive &operator=(mkldnn_primitive &&) = delete;
};

#endif

// vim: et ts=4 sw=4 cindent cino^=l0,\:0,N-s
